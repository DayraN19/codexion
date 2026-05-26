/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bgranier <bgranier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/26 12:22:36 by bgranier          #+#    #+#             */
/*   Updated: 2026/05/26 12:22:37 by bgranier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	handle_wait(t_coder *coder, t_dongle *first, t_dongle *second)
{
	if (heap_peek(first->heap) != coder)
	{
		pthread_mutex_unlock(&second->mutex);
		pthread_cond_wait(&first->cond, &first->mutex);
		pthread_mutex_lock(&second->mutex);
	}
	else if (heap_peek(second->heap) != coder)
	{
		pthread_mutex_unlock(&first->mutex);
		pthread_cond_wait(&second->cond, &second->mutex);
		pthread_mutex_unlock(&second->mutex);
		pthread_mutex_lock(&first->mutex);
		pthread_mutex_lock(&second->mutex);
	}
	else
	{
		pthread_mutex_unlock(&second->mutex);
		pthread_mutex_unlock(&first->mutex);
		usleep(1000);
		pthread_mutex_lock(&first->mutex);
		pthread_mutex_lock(&second->mutex);
	}
}

static int	wait_loop(t_coder *coder, t_dongle *first, t_dongle *second)
{
	while (1)
	{
		if (heap_peek(first->heap) == coder
			&& heap_peek(second->heap) == coder
			&& get_time() >= first->available_at
			&& get_time() >= second->available_at)
			break ;
		if (simulation_stop(coder->data))
		{
			pthread_mutex_unlock(&second->mutex);
			pthread_mutex_unlock(&first->mutex);
			return (1);
		}
		handle_wait(coder, first, second);
	}
	return (0);
}

void	take_two_dongles(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	first = coder->left_dongle;
	second = coder->right_dongle;
	if (first->id > second->id)
	{
		first = coder->right_dongle;
		second = coder->left_dongle;
	}
	pthread_mutex_lock(&first->mutex);
	pthread_mutex_lock(&second->mutex);
	coder->request_time = get_time();
	heap_push(first->heap, coder, coder->data);
	heap_push(second->heap, coder, coder->data);
	if (wait_loop(coder, first, second))
		return ;
	heap_pop(first->heap, coder->data);
	heap_pop(second->heap, coder->data);
}
