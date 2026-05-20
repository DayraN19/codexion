#include "codexion.h"

// 1. Affichage sécurisé des logs (Protégé par le log_mutex)
void print_status(t_coder *coder, char *status)
{
    pthread_mutex_lock(&coder->data->log_mutex);
    
    // On ne print que si la simulation est toujours en cours
    if (!simulation_stop(coder->data))
    {
        long long timestamp = get_time() - coder->data->start_time;
        printf("%lld ms: Coder %d %s\n", timestamp, coder->id, status);
    }
    
    pthread_mutex_unlock(&coder->data->log_mutex);
}

// 2. Logique pour PRENDRE un dongle via le Heap et les Condition Variables
static void accaparer_dongle(t_dongle *dongle, t_coder *coder)
{
    pthread_mutex_lock(&dongle->mutex);

    // On prend notre ticket temporel (pour le mode FIFO)
    coder->request_time = get_time();
    
    // On s'inscrit dans la file d'attente (Heap) du dongle
    heap_push(dongle->heap, coder, coder->data);

    // Tant que je ne suis pas le premier du Heap OU que le cooldown n'est pas expiré...
    while (heap_peek(dongle->heap) != coder || get_time() < dongle->available_at)
    {
        // On s'endort en relâchant le mutex du dongle. 
        // pthread_cond_wait réenclenchera le mutex automatiquement au réveil.
        pthread_cond_wait(&dongle->cond, &dongle->mutex);
        
        // Si la simulation s'est arrêtée pendant notre sommeil, on quitte proprement
        if (simulation_stop(coder->data))
        {
            pthread_mutex_unlock(&dongle->mutex);
            return ;
        }
    }

    // Si on arrive ici, c'est qu'on est au sommet ET que le cooldown est fini.
    // On sort officiellement du Heap car le dongle est à nous !
    heap_pop(dongle->heap, coder->data);
    
    // ATTENTION : On garde le mutex locké ! 
    // Garder le mutex locké signifie "Je garde le dongle physique dans ma main".
}

// 3. Logique pour RELÂCHER un dongle et activer le Cooldown
static void relacher_dongle(t_dongle *dongle, t_data *data)
{
    // On calcule l'instant précis où ce dongle redeviendra disponible
    dongle->available_at = get_time() + data->t_cooldown;
    
    // On libère le verrou physique
    pthread_mutex_unlock(&dongle->mutex);
    
    // On réveille TOUS les codeurs qui attendaient ce dongle 
    // pour qu'ils revérifient s'ils sont devenus les rois du Heap
    pthread_cond_broadcast(&dongle->cond);
}

// 4. La boucle de vie principale de chaque thread Coder
void *coder_routine(void *ptr)
{
    t_coder *coder = (t_coder *)ptr;

    // Si le nombre de compiles requises est à 0, on s'arrête tout de suite
    if (coder->data->nb_compiles_req == 0)
        return (NULL);

    // Cas particulier : Si un seul codeur est à la table, il n'aura jamais 2 dongles.
    // On le fait attendre sagement jusqu'à son burnout.
    if (coder->data->nb_coders == 1)
    {
        smart_sleep(coder->data->t_burnout + 10, coder->data);
        return (NULL);
    }

    while (!simulation_stop(coder->data))
    {
        // --- PHASE 1 : ACQUISITION DES DONGLES ---
        // Pour éviter les verrous mortels (Deadlocks), une astuce classique 
        // consiste à faire lock d'abord le dongle avec le plus petit ID.
        if (coder->left_dongle->id < coder->right_dongle->id)
        {
            accaparer_dongle(coder->left_dongle, coder);
            accaparer_dongle(coder->right_dongle, coder);
        }
        else
        {
            accaparer_dongle(coder->right_dongle, coder);
            accaparer_dongle(coder->left_dongle, coder);
        }

        if (simulation_stop(coder->data))
            break ;

        // --- PHASE 2 : COMPILATION ---
        print_status(coder, "is compiling");
        
        // On met à jour l'heure de notre "dernier repas" (crucial pour le monitor)
        pthread_mutex_lock(&coder->data->dead_mutex);
        coder->last_compile_start = get_time();
        pthread_mutex_unlock(&coder->data->dead_mutex);

        smart_sleep(coder->data->t_compile, coder->data);
        
        coder->nb_compiles++;

        // --- PHASE 3 : LIBÉRATION ---
        // On lâche les dongles (ce qui calcule le cooldown et broadcast)
        relacher_dongle(coder->left_dongle, coder->data);
        relacher_dongle(coder->right_dongle, coder->data);

        // Si le codeur a atteint son quota personnel de compilations requis
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
