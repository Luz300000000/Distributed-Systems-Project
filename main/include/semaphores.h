#ifndef _SEMAPHORES_H
#define _SEMAPHORES_H

struct semaphore_t{
   pthread_mutex_t mutex;
   pthread_cond_t cond;
   int n_read;
   int n_write;
};

struct semaphores_t{
   struct semaphore_t *sem_table;
   struct semaphore_t *sem_stats;
};

#endif