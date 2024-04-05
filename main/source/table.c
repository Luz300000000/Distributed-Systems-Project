/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "table-private.h"

struct table_t *table_create(int n){
    struct table_t *created = NULL;

    if(n > 0){
        created = malloc(sizeof(struct table_t));
        if (created != NULL){
            created->size = n;
            // Vai guardar apenas os apontadores para o endereço das listas (array de endereços de lista)
            created->lists = (struct list_t**) malloc(sizeof(struct list_t*) * n);
            if (created->lists != NULL)
                for (int i = 0; i < n; i++)
                    created->lists[i] = list_create();
        }
    }

    return created;
}

int table_destroy(struct table_t *table){
    int res = -1;

    if(table != NULL) {
        for(int i = 0; i < table->size; i++)
            list_destroy(table->lists[i]);

        free(table->lists);
        table->size = 0;
        free(table);
        res = 0;
    }

    return res;
}

int table_put(struct table_t *table, char *key, struct data_t *value){
    int res = -1;

    if(table != NULL && key != NULL && value != NULL) {
        // Criação de cópias da chave e dos dados
        char* copy_key = strdup(key);
        struct data_t *copy_value = data_dup(value);

        struct entry_t *new_entry = entry_create(copy_key, copy_value);
        if(list_add(table->lists[hash_code(key,table->size)],new_entry) >= 0)
            res = 0;
    }
    
    return res;
}

struct data_t *table_get(struct table_t *table, char *key){
    struct data_t *data = NULL;
    if(table != NULL && key != NULL) {
        struct entry_t *entry = list_get(table->lists[hash_code(key,table->size)],key);
        if(entry != NULL)
            data = data_dup(entry->value);
    }
    return data;
}

int table_remove(struct table_t *table, char *key){
    int res = -1;
    if(table != NULL && key != NULL)
        res = list_remove(table->lists[hash_code(key,table->size)],key);
    return res;    
}

int table_size(struct table_t *table){
    int res = -1;

    if(table != NULL){
        if(table->size >= 0){
            res = 0;
            for(int i = 0; i < table->size; i++)
                res += list_size(table->lists[i]);
        }
    }
    return res;
}

char **table_get_keys(struct table_t *table){
    char **list_keys, **table_keys = NULL;
    int t_size  = table_size(table);

    if (table != NULL && t_size > 0){
        table_keys = malloc((t_size + 1) * sizeof(char*));
        if (table_keys != NULL){
            int j,k = 0;
            // Itera sobre as linhas da table
            for (int i = 0; i < table->size; i++) {
                list_keys = list_get_keys(table->lists[i]);
                j = 0;
                // Se a linha não está vazia entra
                if(list_size(table->lists[i]) != 0)
                    // Percorre a lista de keys da current_line
                    while(list_keys[j] != NULL){
                        table_keys[k] = list_keys[j];
                        j++;
                        k++; // counter total que itera sobre o array da table
                    }
                free(list_keys);
            }
        table_keys[k] = NULL; // Index final tem de sinalizar o fim da lista
        }
    }
    return table_keys;
}

int table_free_keys(char **keys){
    return list_free_keys(keys);
}

int hash_code(char *key, int n){
    int charValueSum = 0;
    for (int i = 0; key[i] != '\0'; i++) 
        charValueSum += key[i];
    return charValueSum % n;
}