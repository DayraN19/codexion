#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <stdlib.h>
# include <stdio.h>
#include <unistd.h>

typedef struct s_heap
{
	int				size;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;

	long long		available_at;

	t_heap			*heap;

}	t_dongle;

struct s_data;

typedef struct s_coder
{
	int				id;

	int				nb_compiles;

	long long		last_compile_start;

	t_dongle		*left_dongle;
	t_dongle		*right_dongle;

	struct s_data	*data;

	pthread_t		thread;

}	t_coder;


typedef struct s_data
{
	int				nb_coders;

	long long		time_to_compile;
	long long		time_to_rest;
	long long		time_to_burnout;

	int				simulation_running;

	pthread_mutex_t	log_mutex;

	t_coder		*coders;
	t_dongle		*dongles;

}	t_data;

#endif