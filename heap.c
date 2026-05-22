#include "codexion.h"

void heap_swap(t_heap *heap, int i, int j)
{
    t_coder *temp;

    temp = heap->data[i];
    heap->data[i] = heap->data[j];
    heap->data[j] = temp;
}

int heap_compare(t_coder *a, t_coder *b, t_data *data)
{
    if(data->scheduler == FIFO)
    {
        if (a->request_time < b->request_time)
            return (1);
        if (a->request_time > b->request_time)
            return (0);
        return (a->id < b->id);
    }
    else
    {
        if (a->last_compile_start < b->last_compile_start)
            return (1);
        if (a->last_compile_start > b->last_compile_start)
            return (0);
        return (a->id < b->id);
    }
}

void heap_push(t_heap *heap, t_coder *coder, t_data *data)
{
    int i;
    int parent;

    if (heap->size >= heap->capacity)
        return ;

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
            break ;
    }
}

void heap_pop(t_heap *heap, t_data *data)
{
    int i = 0;
    int smallest;
    int left, right;

    if (heap->size <= 0)
        return ;
    heap->size--;
    heap->data[0] = heap->data[heap->size];
    while (1)
    {
        smallest = i;
        left = (2 * i) + 1;
        right = (2 * i) + 2;
        if (left < heap->size && heap_compare(heap->data[left], heap->data[smallest], data))
            smallest = left;
        if (right < heap->size && heap_compare(heap->data[right], heap->data[smallest], data))
            smallest = right;
        if (smallest != i)
        {
            heap_swap(heap, i, smallest);
            i = smallest;
        }
        else
            break ;
    }
}

t_coder *heap_peek(t_heap *heap)
{
    if (!heap || heap->size <= 0)
        return (NULL);
    return (heap->data[0]);
}
