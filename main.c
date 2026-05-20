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
    // Initialisation globale
    if (init_all(&data, av) != 0)
    {
        cleanup(&data);
        return (1);
    }
    // Création des threads "Codeurs"
    for (int i = 0; i < data.nb_coders; i++)
    {
        if (pthread_create(&data.coders[i].thread, NULL, &coder_routine, &data.coders[i]) != 0)
        {
            cleanup(&data);
            return (1);
        }
    }
    // Création du thread "Monitor" (le surveillant)
    if (pthread_create(&monitor, NULL, &monitor_routine, &data) != 0)
    {
        cleanup(&data);
        return (1);
    }
    // On attend d'abord que le monitor détecte une fin (mort ou objectif atteint)
    pthread_join(monitor, NULL);
    // On attend ensuite que tous les codeurs s'arrêtent suite au signal du monitor
    for (int i = 0; i < data.nb_coders; i++)
    {
        pthread_join(data.coders[i].thread, NULL);
    }
    // Libération propre de la mémoire
    cleanup(&data);
    return (0);
}