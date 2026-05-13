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
    long long       last_compile_start; // Pour le burnout et l'EDF
    long long       request_time;       // Pour le FIFO (le "ticket" d'entrée)
    pthread_t       thread;
    struct s_dongle *left_dongle;
    struct s_dongle *right_dongle;
    struct s_data   *data;              // Lien vers les paramètres globaux
} t_coder;

typedef struct s_heap {
    t_coder         **data;             // Tableau de pointeurs
    int             size;
    int             capacity;
} t_heap;

typedef struct s_dongle {
    int             id;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    long long       available_at;       // Pour gérer le cooldown
    t_heap          *heap;              // La file d'attente de ce dongle
} t_dongle;

typedef struct s_data {
    int             nb_coders;
    long long       t_burnout;
    long long       t_compile;
    long long       t_debug;
    long long       t_refactor;
    int             nb_compiles_req;
    long long       t_cooldown;
    t_sched         scheduler;          // Ton fameux scheduler (FIFO ou EDF)
    
    long long       start_time;
    bool            is_dead;            // Flag pour arrêter la simulation
    pthread_mutex_t log_mutex;          // Pour protéger les printf
    pthread_mutex_t dead_mutex;         // Pour protéger le flag is_dead
    
    t_coder         *coders;            // Tableau de tous les codeurs
    t_dongle        *dongles;           // Tableau de tous les dongles
} t_data;

// Prototypes (à compléter selon tes besoins)
long long   get_time(void);
int         heap_compare(t_coder *a, t_coder *b, t_data *data);
void 		heap_swap(t_heap *heap, int i, int j);
t_coder 	*heap_peek(t_heap *heap);
void 		heap_pop(t_heap *heap, t_data *data);
bool    	simulation_stop(t_data *data);
void 		heap_push(t_heap *heap, t_coder *coder, t_data *data);

#endif