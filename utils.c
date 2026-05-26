/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgranier <bgranier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 12:25:29 by bgranier          #+#    #+#             */
/*   Updated: 2026/05/26 12:26:36 by bgranier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	simulation_stop(t_data *data)
{
	bool	stop;

	pthread_mutex_lock(&data->dead_mutex);
	stop = data->is_dead;
	pthread_mutex_unlock(&data->dead_mutex);
	return (stop);
}

long long	ft_atoll(const char *str)
{
	long long	res;
	int			i;
	int			sign;

	res = 0;
	i = 0;
	sign = 1;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
			sign = -1;
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		res = res * 10 + (str[i] - '0');
		i++;
	}
	return (res * sign);
}

static void	free_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		pthread_mutex_destroy(&data->dongles[i].mutex);
		pthread_cond_destroy(&data->dongles[i].cond);
		if (data->dongles[i].heap)
		{
			free(data->dongles[i].heap->data);
			free(data->dongles[i].heap);
		}
		i++;
	}
	free(data->dongles);
}

void	cleanup(t_data *data)
{
	if (data->dongles)
		free_dongles(data);
	if (data->coders)
		free(data->coders);
	pthread_mutex_destroy(&data->log_mutex);
	pthread_mutex_destroy(&data->dead_mutex);
}

t_coder	*heap_peek(t_heap *heap)
{
	if (!heap || heap->size <= 0)
		return (NULL);
	return (heap->data[0]);
}
