#include "codexion.h"

// Fonction utilitaire pour vérifier si TOUS les codeurs ont fini leur quota
static bool check_all_compiles_reached(t_data *data)
{
    // Si aucune limite n'est demandée (-1), on ne s'arrête pas pour ça
    if (data->nb_compiles_req == -1)
        return (false);

    for (int i = 0; i < data->nb_coders; i++)
    {
        // Si au moins un codeur n'a pas fini, la simulation continue
        if (data->coders[i].nb_compiles < data->nb_compiles_req)
            return (false);
    }
    return (true);
}

// Boucle principale du thread Monitor
void *monitor_routine(void *ptr)
{
    t_data *data = (t_data *)ptr;

    // Petite sécurité : si on demande 0 compile, on s'arrête direct
    if (data->nb_compiles_req == 0)
        return (NULL);

    while (1)
    {
        for (int i = 0; i < data->nb_coders; i++)
        {
            pthread_mutex_lock(&data->dead_mutex);
            long long time_since_last_compile = get_time() - data->coders[i].last_compile_start;

            // CAS 1 : BURNOUT DETECTÉ
            if (time_since_last_compile > data->t_burnout)
            {
                data->is_dead = true;
                pthread_mutex_unlock(&data->dead_mutex);

                // On lock les logs pour afficher la mort de manière synchrone
                pthread_mutex_lock(&data->log_mutex);
                long long timestamp = get_time() - data->start_time;
                printf("%lld ms: Coder %d died of burnout\n", timestamp, data->coders[i].id);
                pthread_mutex_unlock(&data->log_mutex);
                
                return (NULL);
            }
            pthread_mutex_unlock(&data->dead_mutex);
        }

        // CAS 2 : TOUT LE MONDE A FINI
        if (check_all_compiles_reached(data))
        {
            pthread_mutex_lock(&data->dead_mutex);
            data->is_dead = true;
            pthread_mutex_unlock(&data->dead_mutex);
            return (NULL);
        }

        // On fait une micro-pause (ex: 1ms) pour éviter de surcharger le CPU à 100%
        usleep(1000);
    }
    return (NULL);
}