#include "codexion.h"

long long	get_time(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);

	return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

void	smart_sleep(long long time, t_data *data)
{
	long long	start;

	start = get_time();

	while (get_time() - start < time)
	{
		if (!data->t_compile)
			break ;
		usleep(500);
	}
}