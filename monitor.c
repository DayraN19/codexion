#include "codexion.h"

static bool check_all_compiles_reached(t_data *data)
{
	if (data->nb_compiles_req == -1)
		return false;

	for (int i = 0; i < data->nb_coders; i++)
	{
		if (data->coders[i].nb_compiles < data->nb_compiles_req)
			return false;
	}
	return true;
}

void *monitor_routine(void *ptr)
{
	t_data *data = (t_data *)ptr;

	while (!simulation_stop(data))
	{
		for (int i = 0; i < data->nb_coders; i++)
		{
			long long now = get_time();
			long long last;

			pthread_mutex_lock(&data->dead_mutex);
			last = data->coders[i].last_compile_start;

			if (!data->is_dead && (now - last > data->t_burnout))
			{
				data->is_dead = true;

				pthread_mutex_lock(&data->log_mutex);
				printf("%lld ms: Coder %d died of burnout\n",
					now - data->start_time,
					data->coders[i].id);
				pthread_mutex_unlock(&data->log_mutex);

				pthread_mutex_unlock(&data->dead_mutex);
				return NULL;
			}
			pthread_mutex_unlock(&data->dead_mutex);
		}

		if (check_all_compiles_reached(data))
		{
			pthread_mutex_lock(&data->dead_mutex);
			data->is_dead = true;
			pthread_mutex_unlock(&data->dead_mutex);
			return NULL;
		}

		usleep(1000);
	}
	return NULL;
}