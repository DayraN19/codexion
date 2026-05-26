#include "codexion.h"

bool simulation_stop(t_data *data)
{
    bool stop;

    pthread_mutex_lock(&data->dead_mutex);
    stop = data->is_dead;
    pthread_mutex_unlock(&data->dead_mutex);
    return (stop);
}

long long ft_atoll(const char *str)
{
    long long res;
    int       i;
    int       sign;

    res = 0;
    i = 0;
    sign = 1;
    while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
        i++;
    if (str[i] == '-' || str[i] == '+')
    {
        if (str[i] == '-')
            sign = -1;
        i++;
    }
    while (str[i] >= '0' && str[i] <= '9')
    {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return (res * sign);
}