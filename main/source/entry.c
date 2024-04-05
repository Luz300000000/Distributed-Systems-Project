/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entry.h"
#include "data.h"

struct entry_t *entry_create(char *key, struct data_t *data){
    struct entry_t *created = NULL;

    if(key != NULL && data != NULL && data->data != NULL && data->datasize > 0){
        created = (struct entry_t *) malloc(sizeof(struct entry_t));
        if(created != NULL){
            created->key = key;
            created->value = data;
        }
    }

    return created;
}

int entry_destroy(struct entry_t *entry){
    int res = -1;

    if(entry != NULL && entry->key != NULL){
        if(data_destroy(entry->value) == 0){
            free(entry->key);
            free(entry);
            res = 0;
        }
    }
    
    return res;
}

struct entry_t *entry_dup(struct entry_t *entry){
    struct entry_t *dup = NULL;
    
    if(entry != NULL && entry->key != NULL && entry->value != NULL){
        dup = (struct entry_t *) malloc(sizeof(struct entry_t));
        if(dup != NULL){
            dup->key = strdup(entry->key);
            dup->value = data_dup(entry->value);
        }
    }

    return dup;
}

int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    int res = -1;
    
    if(entry != NULL && new_key != NULL && new_value != NULL && entry->key != NULL && entry->value != NULL){
        free(entry->key);
        entry->key = new_key;

        data_destroy(entry->value);
        entry->value = new_value;
        res = 0;
    }
    return res;
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
    int res = -2;
    
    if(entry1 != NULL && entry1->key != NULL && entry2 != NULL && entry2->key != NULL){
        int cmp = strcmp(entry1->key, entry2->key);
        
        if(cmp == 0)
            res = 0;
        else if (cmp < 0)
            res = -1;
        else if(cmp > 0)
            res = 1;
    }
    
    return res;
}
