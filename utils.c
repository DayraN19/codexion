#include "codexion.h"

bool simulation_stop(t_data *data)
{
    bool stop;

    pthread_mutex_lock(&data->dead_mutex);
    stop = data->is_dead;
    pthread_mutex_unlock(&data->dead_mutex);
    return (stop);
}