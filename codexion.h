#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <stdbool.h>

typedef enum e_sched {
	FIFO,
	EDF
} t_sched;

struct s_data;

typedef struct s_coder {
	int             id;
	int             nb_compiles;
	long long       last_compile_start;
	long long       request_time;
	pthread_t       thread;
	struct s_dongle *left_dongle;
	struct s_dongle *right_dongle;
	struct s_data   *data;
} t_coder;

typedef struct s_heap {
	t_coder         **data;
	int             size;
	int             capacity;
} t_heap;

typedef struct s_dongle {
	int             id;
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	long long       available_at;
	t_heap          *heap;
} t_dongle;

typedef struct s_data {
	int             nb_coders;
	long long       t_burnout;
	long long       t_compile;
	long long       t_debug;
	long long       t_refactor;
	int             nb_compiles_req;
	long long       t_cooldown;
	t_sched         scheduler;
	
	long long       start_time;
	bool            is_dead;
	pthread_mutex_t log_mutex;
	pthread_mutex_t dead_mutex;
	
	t_coder         *coders;
	t_dongle        *dongles;
} t_data;


void        heap_swap(t_heap *heap, int i, int j);
int         heap_compare(t_coder *a, t_coder *b, t_data *data);
void        heap_push(t_heap *heap, t_coder *coder, t_data *data);
void        heap_pop(t_heap *heap, t_data *data);
t_coder     *heap_peek(t_heap *heap);
void        heap_take(t_dongle *dongle, t_coder *coder);

int         init_all(t_data *data, char **av);
void        cleanup(t_data *data);

long long   get_time(void);
void        smart_sleep(long long time_to_wait, t_data *data);
bool        simulation_stop(t_data *data);

void        *coder_routine(void *ptr);
void        *monitor_routine(void *ptr);
void        print_status(t_coder *coder, char *status);
long long 	ft_atoll(const char *str);

#endif