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
    if (data->scheduler == FIFO)
    {
        if (a->request_time != b->request_time)
            return (a->request_time < b->request_time);
    }
    else // Mode EDF
    {
        long long deadline_a = a->last_compile_start + data->t_burnout;
        long long deadline_b = b->last_compile_start + data->t_burnout;
        
        if (deadline_a != deadline_b)
            return (deadline_a < deadline_b);
    }

    // TIE-BREAKER (Déterminisme) : Si égalité parfaite, le plus petit ID gagne
    return (a->id < b->id);
}

void heap_push(t_heap *heap, t_coder *coder, t_data *data)
{
    int i;
    int parent;

    if (heap->size >= heap->capacity)
        return ; // Le tas est plein (ne devrait pas arriver ici)

    // On place le codeur à la fin
    i = heap->size;
    heap->data[i] = coder;
    heap->size++;

    // BUBBLE UP : On remonte
    while (i > 0)
    {
        parent = (i - 1) / 2;
        // Si le codeur actuel est plus prioritaire que son parent
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

    // On remplace la racine par le dernier élément
    heap->size--;
    heap->data[0] = heap->data[heap->size];

    // BUBBLE DOWN : On descend
    while (true)
    {
        smallest = i;
        left = (2 * i) + 1;
        right = (2 * i) + 2;

        // On cherche le plus prioritaire entre le parent, l'enfant gauche et l'enfant droit
        if (left < heap->size && heap_compare(heap->data[left], heap->data[smallest], data))
            smallest = left;
        if (right < heap->size && heap_compare(heap->data[right], heap->data[smallest], data))
            smallest = right;

        // Si le plus prioritaire n'est pas le parent, on swap et on continue
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
    if (heap == NULL || heap->size <= 0)
        return (NULL);
    return (heap->data[0]);
}
