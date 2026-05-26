/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgranier <bgranier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 12:15:08 by bgranier          #+#    #+#             */
/*   Updated: 2026/05/26 12:18:01 by bgranier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static bool	check_all_compiles_reached(t_data *data)
{
	int	i;

	i = 0;
	if (data->nb_compiles_req == -1)
		return (false);
	while (i < data->nb_coders)
	{
		if (data->coders[i].nb_compiles < data->nb_compiles_req)
			return (false);
		i++;
	}
	return (true);
}

static int	check_coders(t_data *data)
{
	int			i;
	long long	now;
	long long	last;

	i = 0;
	while (i < data->nb_coders)
	{
		now = get_time();
		pthread_mutex_lock(&data->dead_mutex);
		last = data->coders[i].last_compile_start;
		if (!data->is_dead && (now - last > data->t_burnout))
		{
			data->is_dead = true;
			pthread_mutex_lock(&data->log_mutex);
			printf("%lld %d burned out\n",
				now - data->start_time, data->coders[i].id);
			pthread_mutex_unlock(&data->log_mutex);
			pthread_mutex_unlock(&data->dead_mutex);
			return (1);
		}
		pthread_mutex_unlock(&data->dead_mutex);
		i++;
	}
	return (0);
}

void	*monitor_routine(void *ptr)
{
	t_data	*data;

	data = (t_data *)ptr;
	while (!simulation_stop(data))
	{
		if (check_coders(data))
			return (NULL);
		if (check_all_compiles_reached(data))
		{
			pthread_mutex_lock(&data->dead_mutex);
			data->is_dead = true;
			pthread_mutex_unlock(&data->dead_mutex);
			return (NULL);
		}
		usleep(1000);
	}
	return (NULL);
}
