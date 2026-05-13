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

int main()
{
	printf("ntm\n");
}