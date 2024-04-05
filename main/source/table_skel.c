/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "table_skel.h"
#include "table_skel-private.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"

extern struct rtable_t *rtable_write;

struct table_t *table_skel_init(int n_lists){
    return table_create(n_lists);
}

int table_skel_destroy(struct table_t *table){
    return table_destroy(table);
}

struct statistics_t *table_skel_stats_init(){
    struct statistics_t *stats = malloc(sizeof(struct statistics_t));
    return stats;
}

struct semaphores_t *table_skel_semaphores_init(){
    struct semaphores_t *sem = malloc(sizeof(struct semaphores_t));

    sem->sem_stats = malloc(sizeof(struct semaphore_t));
    pthread_mutex_init(&sem->sem_stats->mutex,NULL);
    pthread_cond_init(&sem->sem_stats->cond,NULL);

    sem->sem_table = malloc(sizeof(struct semaphore_t));
    pthread_mutex_init(&sem->sem_table->mutex,NULL);
    pthread_cond_init(&sem->sem_table->cond,NULL);

    return sem;
}

int invoke(MessageT *msg, struct table_t *table, struct statistics_t *stats, struct semaphores_t *sem){
    int res = 0;
    int table_op = 1;
    struct timeval initial, final;
    MessageT__Opcode op = msg->opcode;

    switch(op){
        case MESSAGE_T__OPCODE__OP_PUT: // PUT
            gettimeofday(&initial,NULL);
            res = op_put(msg,table,sem->sem_table);
            gettimeofday(&final,NULL);

            if(rtable_write != NULL)
                network_send_receive(rtable_write,msg);
            break;
        case MESSAGE_T__OPCODE__OP_GET: // GET
            gettimeofday(&initial,NULL);
            res = op_get(msg,table,sem->sem_table);
            gettimeofday(&final,NULL);
            break;
        case MESSAGE_T__OPCODE__OP_DEL: // DEL
            gettimeofday(&initial,NULL);
            res = op_del(msg,table,sem->sem_table);
            gettimeofday(&final,NULL);

            if(rtable_write != NULL)
                network_send_receive(rtable_write,msg);
            break;
        case MESSAGE_T__OPCODE__OP_SIZE: // SIZE
            gettimeofday(&initial,NULL);
            res = op_size(msg,table,sem->sem_table);
            gettimeofday(&final,NULL);
            break;
        case MESSAGE_T__OPCODE__OP_GETKEYS: // GETKEYS
            gettimeofday(&initial,NULL);
            res = op_getkeys(msg,table,sem->sem_table);
            gettimeofday(&final,NULL);
            break;
        case MESSAGE_T__OPCODE__OP_GETTABLE: // GETTABLE
            gettimeofday(&initial,NULL);
            res = op_gettable(msg,table,sem->sem_table);
            gettimeofday(&final,NULL);
            break;
        case MESSAGE_T__OPCODE__OP_STATS: // STATS
            res = op_stats(msg,stats,sem->sem_stats);
            table_op = 0;
            break;
        default:
            res = -1;
            table_op = 0;
            break;
    }

    if(res != 0){
        msg->opcode = MESSAGE_T__OPCODE__OP_ERROR;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    }
    else{
        msg->opcode++;
    }

    if(table_op){
        enter_write(sem->sem_stats);
        stats->operations++;
        stats->time_spent += (final.tv_sec-initial.tv_sec)*1000000+(final.tv_usec-initial.tv_usec);
        leave_write(sem->sem_stats);
    }

    return res;
}

int op_put(MessageT *msg, struct table_t *table, struct semaphore_t *sem){
    enter_write(sem);

    int size_p = msg->entry->value.len;
    void *raw_data = malloc(size_p);
    memcpy(raw_data,msg->entry->value.data,size_p);

    struct data_t *data_p = data_create(size_p,raw_data);

    int res = table_put(table,msg->entry->key,data_p);

    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
    data_destroy(data_p);

    leave_write(sem);
    return res;
}

int op_get(MessageT *msg, struct table_t *table, struct semaphore_t *sem){
    enter_read(sem);

    struct data_t *data_g = table_get(table,msg->key);
    int res = -1;

    if (data_g != NULL){
        res = 0;
        msg->value.len = data_g->datasize;
        msg->value.data = malloc(data_g->datasize);
        memcpy(msg->value.data,data_g->data,data_g->datasize);
        msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
    }
    data_destroy(data_g);

    leave_read(sem);
    return res;
}

int op_del(MessageT *msg, struct table_t *table, struct semaphore_t *sem){
    enter_write(sem);
    
    int res = table_remove(table,msg->key);
    msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;

    leave_write(sem);
    return res;
}

int op_size(MessageT *msg, struct table_t *table, struct semaphore_t *sem){
    enter_read(sem);

    int res = table_size(table);
    if(res == -1)
        return res;

    msg->result = res;
    msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;

    leave_read(sem);
    return 0;
}

int op_getkeys(MessageT *msg, struct table_t *table, struct semaphore_t *sem){
    enter_read(sem);

    char **keys = table_get_keys(table);

    if(keys == NULL){
        msg->keys = malloc(1);
        msg->n_keys = 0;
    } else {
        int n_keys = 0;
        while(keys[n_keys] != NULL)
            n_keys++;
                    
        msg->keys = malloc((n_keys + 1) * sizeof(char *));
        msg->n_keys = n_keys;

        for (int j = 0; j < n_keys; j++) {
            msg->keys[j] = malloc(strlen(keys[j]) + 1);
            strcpy(msg->keys[j], keys[j]);
        }
    }

    table_free_keys(keys);
    msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;

    leave_read(sem);
    return 0;
}

int op_gettable(MessageT *msg, struct table_t *table, struct semaphore_t *sem){
    enter_read(sem);

    char **key_list = table_get_keys(table);
    if(key_list != NULL){
        msg->n_entries = table_size(table);
        msg->entries = malloc(sizeof(EntryT *) * msg->n_entries);

        for(int i = 0; key_list[i] != NULL; i++){
            struct data_t *data = table_get(table,key_list[i]);

            EntryT *entry = malloc(sizeof(EntryT));
            entry_t__init(entry);

            entry->key = malloc(strlen(key_list[i])+1);
            strcpy(entry->key,key_list[i]);

            entry->value.len = data->datasize;

            entry->value.data = malloc(data->datasize);
            memcpy(entry->value.data,data->data,data->datasize);

            msg->entries[i] = entry;

            data_destroy(data);
        }
    } else{
        msg->entries = malloc(1);
        msg->n_keys = 0;
    }

    table_free_keys(key_list);
    msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
    
    leave_read(sem);
    return 0;
}

int op_stats(MessageT *msg, struct statistics_t *stats, struct semaphore_t *sem){
    enter_read(sem);

    if(stats == NULL){
        msg->stats = malloc(1);
        return -1;
    }

    StatsT *statsT = malloc(sizeof(StatsT));
    stats_t__init(statsT);

    statsT->operations = stats->operations;
    statsT->time_spent = stats->time_spent;
    statsT->clients = stats->clients;

    msg->stats = statsT;

    msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;

    leave_read(sem);
    return 0;
}

void enter_read(struct semaphore_t *sem){
    pthread_mutex_lock(&sem->mutex);
    while(sem->n_write > 0){
        pthread_cond_wait(&sem->cond,&sem->mutex);
    }
    sem->n_read++;
    pthread_mutex_unlock(&sem->mutex);
}

void leave_read(struct semaphore_t *sem){
    pthread_mutex_lock(&sem->mutex);
    sem->n_read--;
    if(sem->n_read == 0){
        pthread_cond_broadcast(&sem->cond);
    }
    pthread_mutex_unlock(&sem->mutex);
}

void enter_write(struct semaphore_t *sem){
    pthread_mutex_lock(&sem->mutex);
    while(sem->n_write > 0 || sem->n_read > 0){
        pthread_cond_wait(&sem->cond,&sem->mutex);
    }
    sem->n_write++;
    pthread_mutex_unlock(&sem->mutex);
}

void leave_write(struct semaphore_t *sem){
    pthread_mutex_lock(&sem->mutex);
    sem->n_write--;
    pthread_cond_broadcast(&sem->cond);
    pthread_mutex_unlock(&sem->mutex);
}