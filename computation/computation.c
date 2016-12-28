#include "thread_pool.h"
#include "Computation.h"
#include <pthread.h>

void thpool_submit_computation(struct ThreadPool *pool, struct Computation *computation,
                               OnComputationComplete on_complete, void* on_complete_arg)
{
    computation -> on_complete = on_complete;
	computation -> on_complete_arg = on_complete_arg;
	computation -> finished = 0;
	computation -> task.f = computation -> f;
	computation -> task.arg = computation -> arg;
	pthread_mutex_init(&computation -> m, NULL);
	pthread_cond_init(&computation -> signal, NULL);
	thpool_submit(pool, &computation -> task);
}

void thpool_complete_computation(struct Computation *computation)
{
	pthread_mutex_lock(&computation -> m);
	if(computation -> on_complete)
		computation -> on_complete(computation -> on_complete_arg);
	computation -> finished = 1;
	pthread_cond_signal(&computation -> signal);
    pthread_mutex_unlock(&computation -> m);
}

void thpool_wait_computation(struct Computation *computation){
	pthread_mutex_lock(&computation -> m);
	while (!computation -> finished)
	{
		pthread_cond_wait(&computation -> signal, &computation -> m);
	}
	pthread_mutex_unlock(&computation -> m);
	pthread_mutex_destroy(&computation -> m);
	pthread_cond_destroy(&computation -> signal);
	thpool_wait(&computation -> task);

}
