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

    // 1. On prend notre ticket temporel unique pour la demande
    coder->request_time = get_time();
    
    // 2. On s'inscrit UNE SEULE FOIS dans les files d'attente
    heap_push(first->heap, coder, coder->data);
    heap_push(second->heap, coder, coder->data);

    // 3. Boucle d'attente de priorité et de cooldown
    while (heap_peek(first->heap) != coder || heap_peek(second->heap) != coder 
           || get_time() < first->available_at || get_time() < second->available_at)
    {
        // On relâche le second verrou pour éviter de bloquer un autre voisin
        pthread_mutex_unlock(&second->mutex);
        
        // On s'endort sur la condition du premier (ce qui relâche first->mutex automatiquement)
        pthread_cond_wait(&first->cond, &first->mutex);
        
        // Au réveil, on reprend immédiatement le second verrou pour sécuriser les vérifications du while
        pthread_mutex_lock(&second->mutex);
        
        if (simulation_stop(coder->data))
        {
            // Si la simulation s'arrête (mort d'un autre), on doit enlever nos requêtes pour éviter les fuites
            // (Optionnel car le programme s'arrête, mais propre pour Valgrind)
            pthread_mutex_unlock(&second->mutex);
            pthread_mutex_unlock(&first->mutex);
            return ;
        }
    }

    // 4. On a les deux dongles ET on est prioritaires ! 
    // On sort des files d'attente (tas) pour libérer la place aux suivants pendant qu'on compile
    heap_pop(first->heap, coder->data);
    heap_pop(second->heap, coder->data);
    
    // TRÈS IMPORTANT : On garde first->mutex et second->mutex VERROUILLÉS pendant la compilation !
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
        // --- PHASE 1 : ACQUISITION ---
        take_two_dongles(coder);
        if (simulation_stop(coder->data))
            break ;

        // --- PHASE 2 : COMPILATION ---
        print_status(coder, "is compiling");
        
        pthread_mutex_lock(&coder->data->dead_mutex);
        coder->last_compile_start = get_time();
        pthread_mutex_unlock(&coder->data->dead_mutex);

        smart_sleep(coder->data->t_compile, coder->data);
        coder->nb_compiles++;

        // --- PHASE 3 : LIBÉRATION ---
        relacher_les_deux_dongles(coder);

        if (coder->data->nb_compiles_req != -1 && coder->nb_compiles >= coder->data->nb_compiles_req)
            break ;

        if (simulation_stop(coder->data))
            break ;

        // --- PHASE 4 : DEBUGGING ---
        print_status(coder, "is debugging");
        smart_sleep(coder->data->t_debug, coder->data);

        if (simulation_stop(coder->data))
            break ;

        // --- PHASE 5 : REFACTORING ---
        print_status(coder, "is refactoring");
        smart_sleep(coder->data->t_refactor, coder->data);
    }
    return (NULL);
}
