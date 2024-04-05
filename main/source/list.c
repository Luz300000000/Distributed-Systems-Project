/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "list-private.h"

struct list_t *list_create(){
    struct list_t *created = (struct list_t *) malloc(sizeof(struct list_t));
    if (created != NULL){
        created->size = 0;
        created->head = NULL;
    }
    return created;
}

int list_destroy(struct list_t *list){
    int res = -1;

    if(list != NULL){
        struct node_t *current_node = list->head;
        while(current_node != NULL){
            entry_destroy(current_node->entry);
            current_node = get_node(current_node);
        }

        list->size = 0;
        free(list);
        res = 0;
    }

    return res;
}

int list_add(struct list_t *list, struct entry_t *entry){
    int res = -1;
    if(list != NULL && entry != NULL){
        // Verificar se a entry já existe na lista
        struct entry_t *exists = list_get(list,entry->key);
        res = exists != NULL ? list_add_exists(list,entry,exists) : list_add_new(list,entry);
    }
    return res;
}

int list_remove(struct list_t *list, char *key){
    int res = -1;
    if(list != NULL && key != NULL){
        struct entry_t *exists = list_get(list,key);
        res = exists != NULL ? list_remove_exists(list,key) : 1;
    }
    return res;
}

struct entry_t *list_get(struct list_t *list, char *key){
    struct entry_t *entry = NULL;
    if(list != NULL && key != NULL){
        struct node_t *current_node = list->head;
        while(current_node != NULL){
            if(strcmp(current_node->entry->key,key) == 0){
                entry = current_node->entry;
                current_node = NULL; // Sair do ciclo
            } else
                current_node = current_node->next;
        }
    }
    return entry;
}

int list_size(struct list_t *list){
    int res = -1;
    if(list != NULL)
        res = list->size < 0 ? -1 : list->size;

    return res;
}

char **list_get_keys(struct list_t *list){
    char** key_list = NULL;
    if(list != NULL && list->head != NULL){
        key_list = malloc((list->size+1) * sizeof(char*));
        if(key_list != NULL){
            int i = 0;
            struct node_t *current_node = list->head;

            while(current_node != NULL){
                char* current_key = current_node->entry->key;
                key_list[i] = strdup(current_key);
                current_node = current_node->next;
                i++;
            }
            key_list[(list->size)] = NULL; // Index final tem que sinalizar o fim da lista
        }
    }
    return key_list;
}

int list_free_keys(char **keys){
    int res = -1;
    if(keys != NULL){
        int i = 0;
        while(keys[i] != NULL){
            free(keys[i]);
            i++;
        }
        free(keys);
        res = 0;
    }
    return res;
}

// ------------ FUNÇÕES AUXILIARES ------------

struct node_t *get_node(struct node_t *node){
    struct node_t *next_node = NULL;
    if(node != NULL){
        next_node = node->next;
        free(node);
    }
    return next_node;
}

int list_add_exists(struct list_t *list, struct entry_t *new_entry, struct entry_t *old_entry){
    int res = -1;
    struct node_t *current_node = list->head;

    while(current_node != NULL){
        // Temos de voltar a encontrar manualmente a entry para linkar a new_entry ao node
        if(entry_compare(current_node->entry,old_entry) == 0){
            entry_destroy(current_node->entry);
            current_node->entry = new_entry;
            res = 1;
            current_node = NULL; // Sair do ciclo
        } else
            current_node = current_node->next;
    }  

    return res;
}

int list_add_new(struct list_t *list, struct entry_t *entry){
    int res = -1;
    struct node_t *current_node = list->head;

    struct node_t *new_node = malloc(sizeof(struct node_t));
    new_node->entry = entry;

    if(list->size == 0){    // Lista vazia
        list->head = new_node;
        new_node->next = NULL;
        list->size += 1;
        res = 0;
    } else{
        struct node_t *prev_node = NULL;

        while(current_node != NULL){
            if(entry_compare(entry,current_node->entry) == -1){  // entry < current_entry
                if(prev_node == NULL){      // Cabeça da lista
                    new_node->next = current_node;
                    list->head = new_node;
                } else{                     // Meio da lista
                    new_node->next = current_node;
                    prev_node->next = new_node;
                }
                list->size += 1;
                res = 0;
                current_node = NULL; // Sair do ciclo
            } else if(current_node->next == NULL){  // entry > todas as entries, último node
                new_node->next = NULL;
                current_node->next = new_node;
                list->size += 1;
                res = 0;
                current_node = NULL; // Sair do ciclo
            } else{
                prev_node = current_node;
                current_node = current_node->next;
            }
        }
    }

    return res;
}

int list_remove_exists(struct list_t *list, char *key){
    int res = -1;
    struct node_t *current_node = list->head;
    struct node_t *prev_node = NULL;

    while(current_node != NULL){
        if(strcmp(current_node->entry->key,key) == 0){
            if(current_node == list->head && current_node->next == NULL) // Único elemento
                list->head = NULL;
            else if(current_node == list->head) // Cabeça da lista
                list->head = current_node->next;
            else if(current_node->next == NULL) // Cauda da lista
                prev_node->next = NULL;
            else                                // Meio da lista
                prev_node->next = current_node->next;

            entry_destroy(current_node->entry);
            current_node->next = NULL;
            free(current_node);
            current_node = NULL;
            list->size -= 1;
            res = 0;
        } else{
            prev_node = current_node;
            current_node = current_node->next;
        }
    }

    return res;
}