#include "codexion.h"

static int	is_positive_number(char *str)
{
	int	i;

	i = 0;
	if (!str || !str[0])
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

int	check_args(int ac, char **av)
{
	int	i;

	if (ac != 6)
		return (printf("Error: invalid number of arguments\n"), 0);
	i = 1;
	while (i < 5)
	{
		if (!is_positive_number(av[i]))
			return (printf("Error: arguments must be positive numbers\n"), 0);
		i++;
	}
	if (strcmp(av[5], "fifo") != 0
		&& strcmp(av[5], "edf") != 0)
		return (printf("Error: scheduler must be fifo or edf\n"), 0);
	return (1);
}

int main(int ac, char **av)
{
    t_data    data;
    pthread_t monitor;

    // 1. Vérification des arguments (étape cruciale !)
    if (ac != 9)
        return (printf("Error: Wrong number of arguments\n"), 1);
    
    // 2. Initialisation des structures
    // On passe l'adresse de data pour que init_all puisse la remplir
    if (init_all(&data, av) != 0)
        return (1);

    // 3. Lancement des threads Codeurs
    for (int i = 0; i < data.nb_coders; i++)
    {
        if (pthread_create(&data.coders[i].thread, NULL, &coder_routine, &data.coders[i]) != 0)
            return (cleanup(&data), 1);
    }

    // 4. Lancement du thread Monitor
    if (pthread_create(&monitor, NULL, &monitor_routine, &data) != 0)
        return (cleanup(&data), 1);

    // 5. Attente de la fin de la simulation
    // On attend d'abord que le monitor voit un mort ou la fin des compiles
    pthread_join(monitor, NULL);
    
    // On attend que chaque codeur s'arrête proprement
    for (int i = 0; i < data.nb_coders; i++)
        pthread_join(data.coders[i].thread, NULL);

    // 6. Libération de la mémoire
    cleanup(&data);

    return (0);
}