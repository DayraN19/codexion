#include "codexion.h"

int init_data(t_data *data, int ac, char **av)
{
    data->nb_coders = atoi(av[1]);
    data->t_burnout = atoll(av[2]);
    // ... remplis les autres temps (compile, debug, etc.) ...
    
    data->is_dead = false;
    data->start_time = get_time(); // On fixe le point 0 de la simulation

    // Initialisation des mutex globaux
    pthread_mutex_init(&data->log_mutex, NULL);
    pthread_mutex_init(&data->dead_mutex, NULL);

    // Allocation des tableaux
    data->coders = malloc(sizeof(t_coder) * data->nb_coders);
    data->dongles = malloc(sizeof(t_dongle) * data->nb_coders);
    
    if (!data->coders || !data->dongles)
        return (1);
    return (0);
}

void init_dongles(t_data *data)
{
    for (int i = 0; i < data->nb_coders; i++)
    {
        data->dongles[i].id = i;
        data->dongles[i].available_at = 0;
        pthread_mutex_init(&data->dongles[i].mutex, NULL);
        pthread_cond_init(&data->dongles[i].cond, NULL);
        
        // Initialisation du tas (Heap) pour ce dongle
        data->dongles[i].heap = malloc(sizeof(t_heap));
        data->dongles[i].heap->capacity = data->nb_coders;
        data->dongles[i].heap->size = 0;
        data->dongles[i].heap->data = malloc(sizeof(t_coder *) * data->nb_coders);
    }
}

void init_coders(t_data *data)
{
    for (int i = 0; i < data->nb_coders; i++)
    {
        data->coders[i].id = i + 1; // ID commence souvent à 1 pour l'affichage
        data->coders[i].nb_compiles = 0;
        data->coders[i].last_compile_start = data->start_time;
        data->coders[i].data = data;
        
        // Attribution des dongles
        data->coders[i].left_dongle = &data->dongles[i];
        data->coders[i].right_dongle = &data->dongles[(i + 1) % data->nb_coders];
    }
}