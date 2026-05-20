#include "codexion.h"

void cleanup(t_data *data)
{
    if (data->dongles)
    {
        for (int i = 0; i < data->nb_coders; i++)
        {
            pthread_mutex_destroy(&data->dongles[i].mutex);
            pthread_cond_destroy(&data->dongles[i].cond);
            if (data->dongles[i].heap)
            {
                free(data->dongles[i].heap->data);
                free(data->dongles[i].heap);
            }
        }
        free(data->dongles);
    }
    if (data->coders)
        free(data->coders);
    pthread_mutex_destroy(&data->log_mutex);
    pthread_mutex_destroy(&data->dead_mutex);
}

static int init_dongles(t_data *data)
{
    for (int i = 0; i < data->nb_coders; i++)
    {
        data->dongles[i].id = i;
        data->dongles[i].available_at = 0;
        if (pthread_mutex_init(&data->dongles[i].mutex, NULL) != 0)
            return (1);
        if (pthread_cond_init(&data->dongles[i].cond, NULL) != 0)
            return (1);
        
        // Allocation du Heap pour chaque dongle
        data->dongles[i].heap = malloc(sizeof(t_heap));
        if (!data->dongles[i].heap)
            return (1);
        data->dongles[i].heap->capacity = data->nb_coders;
        data->dongles[i].heap->size = 0;
        data->dongles[i].heap->data = malloc(sizeof(t_coder *) * data->nb_coders);
        if (!data->dongles[i].heap->data)
            return (1);
    }
    return (0);
}

static void init_coders(t_data *data)
{
    for (int i = 0; i < data->nb_coders; i++)
    {
        data->coders[i].id = i + 1; // ID de 1 à N pour les logs
        data->coders[i].nb_compiles = 0;
        data->coders[i].last_compile_start = data->start_time;
        data->coders[i].request_time = 0;
        data->coders[i].data = data;
        
        // Règle de la table ronde : modulo pour le voisin de droite
        data->coders[i].left_dongle = &data->dongles[i];
        data->coders[i].right_dongle = &data->dongles[(i + 1) % data->nb_coders];
    }
}

int init_all(t_data *data, char **av)
{
    data->nb_coders = atoi(av[1]);
    data->t_burnout = atoll(av[2]);
    data->t_compile = atoll(av[3]);
    data->t_debug = atoll(av[4]);
    data->t_refactor = atoll(av[5]);
    data->nb_compiles_req = atoi(av[6]);
    data->t_cooldown = atoll(av[7]);
    
    if (strcmp(av[8], "fifo") == 0)
        data->scheduler = FIFO;
    else
        data->scheduler = EDF;

    data->is_dead = false;
    data->start_time = get_time();

    if (pthread_mutex_init(&data->log_mutex, NULL) != 0)
        return (1);
    if (pthread_mutex_init(&data->dead_mutex, NULL) != 0)
        return (1);

    data->coders = malloc(sizeof(t_coder) * data->nb_coders);
    data->dongles = malloc(sizeof(t_dongle) * data->nb_coders);
    if (!data->coders || !data->dongles)
        return (1);

    if (init_dongles(data) != 0)
        return (1);
        
    init_coders(data);
    return (0);
}
