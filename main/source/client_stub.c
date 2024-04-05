/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include "inet.h"
#include "client_stub.h"
#include "table.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "stats.h"

struct rtable_t *rtable_connect(char *address_port){
    if(address_port == NULL)
        return NULL;
    
    char *address_port_copy = malloc(strlen(address_port)+1);
    strcpy(address_port_copy,address_port);

    char* address = strtok(address_port_copy,":");
    char* port = strtok(NULL,":");

    struct rtable_t *rtable = malloc(sizeof(struct rtable_t));
    rtable->server_address = malloc(strlen(address)+1);

    strcpy(rtable->server_address,address);
    rtable->server_port = atoi(port);

    if(network_connect(rtable) == -1){
        free(rtable->server_address);
        free(rtable);
        free(address_port_copy);
        return NULL;
    }
    
    free(address_port_copy);
    return rtable;
}

void rtable_connect_server(struct rtable_t *rtable, char *address_port){
    char* address = strtok(address_port,":");
    char* port = strtok(NULL,":");

    rtable->server_address = malloc(strlen(address)+1);

    strcpy(rtable->server_address,address);
    rtable->server_port = atoi(port);

    if(network_connect(rtable) == -1){
        free(rtable->server_address);
        free(rtable);
    }
}

int rtable_disconnect(struct rtable_t *rtable){
    if(rtable == NULL)
        return -1;

    int result = network_close(rtable);

    free(rtable->server_address);
    free(rtable);
    
    if(result == -1)
        return -1;

    return 0;
}

int rtable_put(struct rtable_t *rtable, struct entry_t *entry){
    MessageT msg;
    message_t__init(&msg);

    EntryT sub;
    entry_t__init(&sub);

    msg.opcode = MESSAGE_T__OPCODE__OP_PUT;
    msg.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    sub.key = malloc(strlen(entry->key)+1);
    strcpy(sub.key,entry->key);

    int size = entry->value->datasize;
    sub.value.len = size;

    sub.value.data = malloc(size);
    memcpy(sub.value.data,entry->value->data,size);

    msg.entry = &sub;

    MessageT *received = network_send_receive(rtable,&msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return -1;
    
    free(sub.key);
    free(sub.value.data);
    message_t__free_unpacked(received,NULL);
    return 0;
}

struct data_t *rtable_get(struct rtable_t *rtable, char *key){
    MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_GET;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    msg.key = malloc(strlen(key)+1);
    strcpy(msg.key,key);

    MessageT *received = network_send_receive(rtable,&msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return NULL;

    int size = received->value.len;
    void* raw_data = malloc(size);
    memcpy(raw_data,received->value.data,size);

    struct data_t *data_received = data_create(size,raw_data);

    free(msg.key);
    message_t__free_unpacked(received,NULL);
    return data_received;
}

int rtable_del(struct rtable_t *rtable, char *key){
    MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_DEL;
    msg.c_type = MESSAGE_T__C_TYPE__CT_KEY;

    msg.key = malloc(strlen(key)+1);
    strcpy(msg.key,key);

    MessageT *received = network_send_receive(rtable,&msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return -1;

    free(msg.key);
    message_t__free_unpacked(received,NULL);
    return 0;
}

int rtable_size(struct rtable_t *rtable){
    MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *received = network_send_receive(rtable,&msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return -1;

    int size = received->result;
    message_t__free_unpacked(received,NULL);
    return size;    
}

char **rtable_get_keys(struct rtable_t *rtable){
    MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *received = network_send_receive(rtable,&msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return NULL;

    char **key_list = malloc(sizeof(char*) * received->n_keys + 1);
    for(int i = 0; i < received->n_keys; i++){
        key_list[i] = malloc(strlen(received->keys[i])+1);
        strcpy(key_list[i],received->keys[i]);
    }
    key_list[received->n_keys] = NULL;
    message_t__free_unpacked(received,NULL);
    return key_list;
}

void rtable_free_keys(char **keys){
    table_free_keys(keys);
}

struct entry_t **rtable_get_table(struct rtable_t *rtable){
    MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *received = network_send_receive(rtable,&msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return NULL;

    struct entry_t **entries = malloc(sizeof(struct entry_t *) * (received->n_entries+1));
    for(int i = 0; i < received->n_entries; i++){
        int size = received->entries[i]->value.len;

        //data
        void *raw_data = malloc(size);
        memcpy(raw_data,received->entries[i]->value.data,size);

        struct data_t *data = data_create(size,raw_data);

        //key
        char *key = malloc(strlen(received->entries[i]->key)+1);
        strcpy(key,received->entries[i]->key);

        entries[i] = entry_create(key,data);
    }
    entries[received->n_entries] = NULL;
    message_t__free_unpacked(received,NULL);
    return entries;
}

void rtable_free_entries(struct entry_t **entries){
    if(entries != NULL){
        for (int i = 0; entries[i] != NULL; i++){
            entry_destroy(entries[i]);
        }
        free(entries);
    }
}

struct statistics_t *rtable_stats(struct rtable_t *rtable){
    MessageT msg;
    message_t__init(&msg);

    msg.opcode = MESSAGE_T__OPCODE__OP_STATS;
    msg.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    MessageT *received = network_send_receive(rtable, &msg);
    if(received == NULL || (received->opcode == MESSAGE_T__OPCODE__OP_ERROR && received->c_type == MESSAGE_T__C_TYPE__CT_NONE))
        return NULL;

    struct statistics_t *stats = malloc(sizeof(struct statistics_t));
    
    stats->operations = received->stats->operations;
    stats->time_spent = received->stats->time_spent;
    stats->clients = received->stats->clients;

    message_t__free_unpacked(received,NULL);
    return stats;
}