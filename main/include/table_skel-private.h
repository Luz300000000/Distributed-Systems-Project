#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "table_skel.h"
#include "sdmessage.pb-c.h"

int op_put(MessageT *msg, struct table_t *table, struct semaphore_t *sem);
int op_get(MessageT *msg, struct table_t *table, struct semaphore_t *sem);
int op_del(MessageT *msg, struct table_t *table, struct semaphore_t *sem);
int op_size(MessageT *msg, struct table_t *table, struct semaphore_t *sem);
int op_getkeys(MessageT *msg, struct table_t *table, struct semaphore_t *sem);
int op_gettable(MessageT *msg, struct table_t *table, struct semaphore_t *sem);
int op_stats(MessageT *msg, struct statistics_t *stats, struct semaphore_t *sem);

void enter_read(struct semaphore_t *sem);
void leave_read(struct semaphore_t *sem);

void enter_write(struct semaphore_t *sem);
void leave_write(struct semaphore_t *sem);

#endif