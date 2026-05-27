/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgranier <bgranier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 12:22:28 by bgranier          #+#    #+#             */
/*   Updated: 2026/05/27 11:22:37 by bgranier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	print_status(t_coder *coder, char *status)
{
	long long	timestamp;

	pthread_mutex_lock(&coder->data->log_mutex);
	if (!simulation_stop(coder->data))
	{
		timestamp = get_time() - coder->data->start_time;
		printf("%lld %d %s\n", timestamp, coder->id, status);
	}
	pthread_mutex_unlock(&coder->data->log_mutex);
}

void	relacher_les_deux_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	long long	now;

	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first->id > second->id)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	now = get_time();
	coder->left_dongle->available_at = now + coder->data->t_cooldown;
	coder->right_dongle->available_at = now + coder->data->t_cooldown;
	heap_pop(first->heap, coder->data);
	heap_pop(second->heap, coder->data);
	pthread_cond_broadcast(&first->cond);
	pthread_cond_broadcast(&second->cond);
	pthread_mutex_unlock(&second->mutex);
	pthread_mutex_unlock(&first->mutex);
}

static int	compile_and_work(t_coder *coder)
{
	take_two_dongles(coder);
	if (simulation_stop(coder->data))
		return (1);
	print_status(coder, "is compiling");
	pthread_mutex_lock(&coder->data->dead_mutex);
	coder->last_compile_start = get_time();
	pthread_mutex_unlock(&coder->data->dead_mutex);
	smart_sleep(coder->data->t_compile, coder->data);
	pthread_mutex_lock(&coder->data->dead_mutex);
	coder->nb_compiles++;
	pthread_mutex_unlock(&coder->data->dead_mutex);
	relacher_les_deux_dongles(coder);
	if (coder->data->nb_compiles_req != -1
		&& coder->nb_compiles >= coder->data->nb_compiles_req)
		return (1);
	if (simulation_stop(coder->data))
		return (1);
	print_status(coder, "is debugging");
	smart_sleep(coder->data->t_debug, coder->data);
	if (simulation_stop(coder->data))
		return (1);
	print_status(coder, "is refactoring");
	smart_sleep(coder->data->t_refactor, coder->data);
	return (0);
}

void	*coder_routine(void *ptr)
{
	t_coder	*coder;

	coder = (t_coder *)ptr;
	if (coder->data->nb_compiles_req == 0)
		return (NULL);
	if (coder->data->nb_coders == 1)
	{
		smart_sleep(coder->data->t_burnout + 10, coder->data);
		return (NULL);
	}
	while (!simulation_stop(coder->data))
	{
		if (compile_and_work(coder))
			break ;
	}
	return (NULL);
}
