#include "../include/thread_pool.h"
#include <pthread.h>

typedef void(*OnComputationComplete)(void*);

struct Computation {
	struct Task task;
	int finished;
	pthread_cond_t signal;
	pthread_mutex_t m;
	OnComputationComplete on_complete;
    void* on_complete_arg;
	
    void (*f)(void*);
    void* arg;
};

void thpool_submit_computation(
    struct ThreadPool *pool,
    struct Computation *computation,
    OnComputationComplete on_complete,
    void* on_complete_arg
);

void thpool_complete_computation(struct Computation *computation);

void thpool_wait_computation(struct Computation *computation);
