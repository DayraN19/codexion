#include "codexion.h"

/* ========================= */
/*        UTIL HEAP          */
/* ========================= */

void heap_swap(t_heap *heap, int i, int j)
{
	t_coder *tmp;

	(void)heap;
	tmp = heap->data[i];
	heap->data[i] = heap->data[j];
	heap->data[j] = tmp;
}

/* ========================= */
/*        REMOVE SAFE        */
/* ========================= */

void heap_remove(t_heap *heap, t_coder *coder)
{
	int i = 0;

	while (i < heap->size)
	{
		if (heap->data[i] == coder)
		{
			heap_swap(heap, i, heap->size - 1);
			heap->size--;
			return;
		}
		i++;
	}
}

/* ========================= */
/*        HEAP TAKE          */
/* ========================= */

void heap_take(t_dongle *dongle, t_coder *coder)
{
	pthread_mutex_lock(&dongle->mutex);

	coder->request_time = get_time();
	heap_push(dongle->heap, coder, coder->data);

	while (!simulation_stop(coder->data))
	{
		if (heap_peek(dongle->heap) == coder
			&& get_time() >= dongle->available_at)
			break;

		pthread_cond_wait(&dongle->cond, &dongle->mutex);
	}

	if (simulation_stop(coder->data))
	{
		heap_remove(dongle->heap, coder);
		pthread_mutex_unlock(&dongle->mutex);
		return;
	}

	heap_pop(dongle->heap, coder->data);

	pthread_mutex_unlock(&dongle->mutex);
}

/* ========================= */
/*       HEAP COMPARE       */
/* ========================= */

int heap_compare(t_coder *a, t_coder *b, t_data *data)
{
	if (data->scheduler == FIFO)
	{
		if (a->request_time != b->request_time)
			return (a->request_time < b->request_time);
	}
	else
	{
		long long da = a->last_compile_start + data->t_burnout;
		long long db = b->last_compile_start + data->t_burnout;

		if (da != db)
			return (da < db);
	}

	return (a->id < b->id);
}

/* ========================= */
/*        HEAP PUSH         */
/* ========================= */

void heap_push(t_heap *heap, t_coder *coder, t_data *data)
{
	int i;
	int parent;

	if (heap->size >= heap->capacity)
		return;

	i = heap->size;
	heap->data[i] = coder;
	heap->size++;

	while (i > 0)
	{
		parent = (i - 1) / 2;

		if (heap_compare(heap->data[i], heap->data[parent], data))
		{
			heap_swap(heap, i, parent);
			i = parent;
		}
		else
			break;
	}
}

/* ========================= */
/*        HEAP POP          */
/* ========================= */

void heap_pop(t_heap *heap, t_data *data)
{
	int i = 0;
	int best;
	int l;
	int r;

	(void)data;

	if (heap->size <= 0)
		return;

	heap->size--;
	heap->data[0] = heap->data[heap->size];

	while (1)
	{
		best = i;
		l = 2 * i + 1;
		r = 2 * i + 2;

		if (l < heap->size
			&& heap_compare(heap->data[l], heap->data[best], data))
			best = l;

		if (r < heap->size
			&& heap_compare(heap->data[r], heap->data[best], data))
			best = r;

		if (best != i)
		{
			heap_swap(heap, i, best);
			i = best;
		}
		else
			break;
	}
}

/* ========================= */
/*        HEAP PEEK         */
/* ========================= */

t_coder *heap_peek(t_heap *heap)
{
	if (!heap || heap->size <= 0)
		return (NULL);
	return (heap->data[0]);
}
