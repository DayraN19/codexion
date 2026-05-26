#include "codexion.h"

int main(int ac, char **av)
{
    t_data    data;
    pthread_t monitor;
    if (ac != 9)
    {
        printf("Error: Usage: ./codexion nb_coders t_burnout t_compile t_debug t_refactor nb_compiles_req t_cooldown scheduler\n");
        return (1);
    }
    if (init_all(&data, av) != 0)
    {
        cleanup(&data);
        return (1);
    }
    int i;
    i = 0;
    while(i < data.nb_coders)
    {
        if (pthread_create(&data.coders[i].thread, NULL, &coder_routine, &data.coders[i]) != 0)
        {
            cleanup(&data);
            return (1);
        }
        i++;
    }
    if (pthread_create(&monitor, NULL, &monitor_routine, &data) != 0)
    {
        cleanup(&data);
        return (1);
    }
    pthread_join(monitor, NULL);
    int i;
    i = 0;
    while(i < data.nb_coders)
    {
        pthread_mutex_lock(&data.dongles[i].mutex);
        pthread_cond_broadcast(&data.dongles[i].cond);
        pthread_mutex_unlock(&data.dongles[i].mutex);
        i++;
    }
    int i;
    i = 0;
    while(i < data.nb_coders)
    {
        pthread_join(data.coders[i].thread, NULL);
        i++;
    }
    cleanup(&data);
    return (0);
}