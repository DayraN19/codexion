#include "codexion.h"

void print_status(t_coder *coder, char *status)
{
    pthread_mutex_lock(&coder->data->log_mutex);
    if (!simulation_stop(coder->data))
    {
        long long timestamp = get_time() - coder->data->start_time;
        printf("%lld ms: Coder %d %s\n", timestamp, coder->id, status);
    }
    pthread_mutex_unlock(&coder->data->log_mutex);
}

static void take_two_dongles(t_coder *coder)
{
    t_dongle *first = (coder->left_dongle->id < coder->right_dongle->id) ? coder->left_dongle : coder->right_dongle;
    t_dongle *second = (coder->left_dongle->id < coder->right_dongle->id) ? coder->right_dongle : coder->left_dongle;

    pthread_mutex_lock(&first->mutex);
    pthread_mutex_lock(&second->mutex);
    coder->request_time = get_time();
    heap_push(first->heap, coder, coder->data);
    heap_push(second->heap, coder, coder->data);
    while (true)
    {
        if (heap_peek(first->heap) == coder && heap_peek(second->heap) == coder 
            && get_time() >= first->available_at && get_time() >= second->available_at)
            break ;
        if (simulation_stop(coder->data))
        {
            pthread_mutex_unlock(&second->mutex);
            pthread_mutex_unlock(&first->mutex);
            return ;
        }
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
    heap_pop(first->heap, coder->data);
    heap_pop(second->heap, coder->data);
}

static void relacher_les_deux_dongles(t_coder *coder)
{
    long long now = get_time();

    coder->left_dongle->available_at = now + coder->data->t_cooldown;
    coder->right_dongle->available_at = now + coder->data->t_cooldown;
    
    pthread_mutex_unlock(&coder->left_dongle->mutex);
    pthread_mutex_unlock(&coder->right_dongle->mutex);
    
    pthread_cond_broadcast(&coder->left_dongle->cond);
    pthread_cond_broadcast(&coder->right_dongle->cond);
}

void *coder_routine(void *ptr)
{
    t_coder *coder = (t_coder *)ptr;

    if (coder->data->nb_compiles_req == 0)
        return (NULL);
    if (coder->data->nb_coders == 1)
    {
        smart_sleep(coder->data->t_burnout + 10, coder->data);
        return (NULL);
    }
    while (!simulation_stop(coder->data))
    {
        take_two_dongles(coder);
        if (simulation_stop(coder->data))
            break ;
        print_status(coder, "is compiling");
        pthread_mutex_lock(&coder->data->dead_mutex);
        coder->last_compile_start = get_time();
        pthread_mutex_unlock(&coder->data->dead_mutex);
        smart_sleep(coder->data->t_compile, coder->data);
        coder->nb_compiles++;
        relacher_les_deux_dongles(coder);
        if (coder->data->nb_compiles_req != -1 && coder->nb_compiles >= coder->data->nb_compiles_req)
            break ;
        if (simulation_stop(coder->data))
            break ;
        print_status(coder, "is debugging");
        smart_sleep(coder->data->t_debug, coder->data);
        if (simulation_stop(coder->data))
            break ;
        print_status(coder, "is refactoring");
        smart_sleep(coder->data->t_refactor, coder->data);
    }
    return (NULL);
}
