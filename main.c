/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgranier <bgranier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 11:25:11 by bgranier          #+#    #+#             */
/*   Updated: 2026/05/26 11:37:40 by bgranier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	print_usage(void)
{
	printf("Error: Usage: ./codexion nb_coders t_burnout "
		"t_compile t_debug t_refactor nb_compiles_req "
		"t_cooldown scheduler\n");
	return (1);
}

static int	start_threads(t_data *data, pthread_t *monitor)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		if (pthread_create(&data->coders[i].thread, NULL,
				&coder_routine, &data->coders[i]) != 0)
			return (1);
		i++;
	}
	if (pthread_create(monitor, NULL, &monitor_routine, data) != 0)
		return (1);
	return (0);
}

static void	stop_simulation(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		pthread_mutex_lock(&data->dongles[i].mutex);
		pthread_cond_broadcast(&data->dongles[i].cond);
		pthread_mutex_unlock(&data->dongles[i].mutex);
		i++;
	}
	i = 0;
	while (i < data->nb_coders)
	{
		pthread_join(data->coders[i].thread, NULL);
		i++;
	}
}

int	main(int ac, char **av)
{
	t_data		data;
	pthread_t	monitor;

	if (ac != 9)
		return (print_usage());
	if (init_all(&data, av) != 0)
	{
		cleanup(&data);
		return (1);
	}
	if (start_threads(&data, &monitor) != 0)
	{
		cleanup(&data);
		return (1);
	}
	pthread_join(monitor, NULL);
	stop_simulation(&data);
	cleanup(&data);
	return (0);
}
