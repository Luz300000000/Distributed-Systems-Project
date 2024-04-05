/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

struct data_t *data_create(int size, void *data){
    struct data_t *created = NULL;

    if(size > 0 && data != NULL){
        created = (struct data_t*) malloc(sizeof(struct data_t));
        if(created != NULL){
            created->datasize = size;
            created->data = data;
        }
    }

    return created;
}

int data_destroy(struct data_t *data){
    int res = -1;
    if(data != NULL && data->data != NULL){
        free(data->data);
        free(data);
        res = 0;
    }
    else
        res = -1;
    
    return res;
}

struct data_t *data_dup(struct data_t *data){
    struct data_t *dup = NULL;

    if(data != NULL && data->datasize > 0 && data->data != NULL){
        dup = (struct data_t *) malloc(sizeof(struct data_t));
        if(dup != NULL){
            dup->datasize = data->datasize;
            dup->data = malloc(dup->datasize);
            memcpy(dup->data,data->data,dup->datasize);
        }
    }

    return dup;
}

int data_replace(struct data_t *data, int new_size, void *new_data){
    int res = -1;

    if(data != NULL && new_size > 0 && new_data != NULL && data->data != NULL && data->datasize > 0){
        free(data->data);
        data->datasize = new_size;
        data->data = new_data;
        res = 0;
    }

    return res;
}
