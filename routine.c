#include "codexion.h"

void print_status(t_coder *coder, char *status)
{
	pthread_mutex_lock(&coder->data->log_mutex);

	if (!simulation_stop(coder->data))
	{
		long long ts = get_time() - coder->data->start_time;
		printf("%lld ms: Coder %d %s\n", ts, coder->id, status);
	}

	pthread_mutex_unlock(&coder->data->log_mutex);
}

static void take_two_dongles(t_coder *coder)
{
	heap_take(coder->left_dongle, coder);
	if (simulation_stop(coder->data))
		return;
	heap_take(coder->right_dongle, coder);
}

static void release_dongles(t_coder *coder)
{
	long long now = get_time();

	pthread_mutex_lock(&coder->left_dongle->mutex);
	pthread_mutex_lock(&coder->right_dongle->mutex);

	coder->left_dongle->available_at = now + coder->data->t_cooldown;
	coder->right_dongle->available_at = now + coder->data->t_cooldown;

	pthread_mutex_unlock(&coder->left_dongle->mutex);
	pthread_mutex_unlock(&coder->right_dongle->mutex);

	pthread_cond_broadcast(&coder->left_dongle->cond);
	pthread_cond_broadcast(&coder->right_dongle->cond);
}

void *coder_routine(void *ptr)
{
	t_coder *coder = (t_coder *)ptr;

	if (coder->data->nb_compiles_req == 0)
		return NULL;

	while (!simulation_stop(coder->data))
	{
		/* TAKE */
		take_two_dongles(coder);
		if (simulation_stop(coder->data))
			break;

		/* COMPILING */
		pthread_mutex_lock(&coder->data->dead_mutex);
		coder->last_compile_start = get_time();
		pthread_mutex_unlock(&coder->data->dead_mutex);

		print_status(coder, "is compiling");

		smart_sleep(coder->data->t_compile, coder->data);
		coder->nb_compiles++;

		if (coder->data->nb_compiles_req != -1
			&& coder->nb_compiles >= coder->data->nb_compiles_req)
			break;

		/* RELEASE */
		release_dongles(coder);

		/* DEBUG */
		print_status(coder, "is debugging");
		smart_sleep(coder->data->t_debug, coder->data);

		/* REFACTOR */
		print_status(coder, "is refactoring");
		smart_sleep(coder->data->t_refactor, coder->data);
	}

	return NULL;
}