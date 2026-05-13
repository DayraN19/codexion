#include "codexion.h"

void heap_take(t_dongle *dongle, t_coder *coder)
{
    // 1. On verrouille l'accès au dongle
    pthread_mutex_lock(&dongle->mutex);

    // 2. On prend notre "ticket" (le temps de requête)
    coder->request_time = get_time();

    // 3. On entre dans la file d'attente (le tas)
    heap_push(dongle->heap, coder, coder->data);

    // 4. On attend que deux conditions soient réunies :
    //    - Je suis le premier dans le tas (heap_peek == moi)
    //    - Le cooldown est terminé (get_time() >= dongle->available_at)
    while (heap_peek(dongle->heap) != coder || get_time() < dongle->available_at)
    {
        // On attend un signal sans consommer de CPU
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
        
        // Si quelqu'un est mort pendant qu'on attendait, on quitte
        if (simulation_stop(coder->data)) 
        {
            pthread_mutex_unlock(&dongle->mutex);
            return;
        }
    }
    // 5. On sort du tas car on a enfin le dongle en main !
    heap_pop(dongle->heap, coder->data);
}
    // On garde le mutex verrouillé ou on le gère selon ta logique de compilation
    // (Attention : bien réfléchir à quand déverrouiller !)