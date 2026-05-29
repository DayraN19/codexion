/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgranier <bgranier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 11:22:58 by bgranier          #+#    #+#             */
/*   Updated: 2026/05/29 14:01:01 by bgranier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongle_heap(t_dongle *dongle, int nb_coders)
{
	dongle->heap = malloc(sizeof(t_heap));
	if (!dongle->heap)
		return (1);
	dongle->heap->size = 0;
	dongle->heap->capacity = nb_coders + 5;
	dongle->heap->data = malloc(sizeof(t_coder *)
			* dongle->heap->capacity);
	if (!dongle->heap->data)
		return (1);
	return (0);
}

static int	init_dongles(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		data->dongles[i].heap = NULL;
		data->dongles[i].id = i;
		data->dongles[i].available_at = 0;
		if (pthread_mutex_init(&data->dongles[i].mutex, NULL) != 0)
			return (1);
		if (pthread_cond_init(&data->dongles[i].cond, NULL) != 0)
			return (1);
		if (init_dongle_heap(&data->dongles[i], data->nb_coders) != 0)
			return (1);
		i++;
	}
	return (0);
}

static void	init_coders(t_data *data)
{
	int	i;

	i = 0;
	while (i < data->nb_coders)
	{
		data->coders[i].id = i + 1;
		data->coders[i].nb_compiles = 0;
		data->coders[i].last_compile_start = data->start_time;
		data->dongles[i].is_used = 0;
		data->coders[i].request_time = 0;
		data->coders[i].data = data;
		data->coders[i].left_dongle = &data->dongles[i];
		data->coders[i].right_dongle = &data->dongles[(i + 1)
			% data->nb_coders];
		i++;
	}
}

static void	parse_args(t_data *data, char **av)
{
	data->coders = NULL;
	data->dongles = NULL;
	data->nb_coders = atoi(av[1]);
	data->t_burnout = ft_atoll(av[2]);
	data->t_compile = ft_atoll(av[3]);
	data->t_debug = ft_atoll(av[4]);
	data->t_refactor = ft_atoll(av[5]);
	data->nb_compiles_req = atoi(av[6]);
	data->t_cooldown = ft_atoll(av[7]);
	if (strcmp(av[8], "fifo") == 0)
		data->scheduler = FIFO;
	else if (strcmp(av[8], "edf") == 0)
		data->scheduler = EDF;
	else
	{
		write(2, "Error: scheduler must be exactly 'fifo' or 'edf'\n", 49);
		exit(1);
	}
	data->is_dead = false;
	data->start_time = get_time();
}

int	init_all(t_data *data, char **av)
{
	parse_args(data, av);
	if (pthread_mutex_init(&data->log_mutex, NULL) != 0)
		return (1);
	if (pthread_mutex_init(&data->dead_mutex, NULL) != 0)
		return (1);
	data->dongles = malloc(sizeof(t_dongle) * data->nb_coders);
	if (!data->dongles)
		return (1);
	if (init_dongles(data) != 0)
		return (1);
	data->coders = malloc(sizeof(t_coder) * data->nb_coders);
	if (!data->coders)
		return (1);
	init_coders(data);
	return (0);
}
