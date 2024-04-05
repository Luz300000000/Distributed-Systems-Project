/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "signal.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "entry.h"
#include "data.h"
#include "stats.h"
#include "table.h"
#include <zookeeper/zookeeper.h>

#define ZDATALEN 1024 * 1024

typedef struct String_vector zoo_string; 

zhandle_t *zh;
int is_connected;

char *head;
char *tail;

struct rtable_t *rtable_head;
struct rtable_t *rtable_tail;

//---------- FUNÇÕES AUXILIARES ----------------------------------

int exists(char* input) {
    return input != NULL && strlen(input) > 0;
}

void put(struct rtable_t *rtable, char* key, void* data){
    struct data_t *data_e = data_create(sizeof(data),data);
    struct entry_t *entry = entry_create(key,data_e);

    int res = rtable_put(rtable,entry);
    if (res != 0) {
        printf("Error in rtable_put!\n");
        fflush(stdout);
    }
    free(entry->value);
    free(entry);
}

void get(struct rtable_t *rtable, char* key){    
    struct data_t *data = rtable_get(rtable,key);

    if (data == NULL) {
        printf("Error in rtable_get or key not found!\n");
    } else {
        printf("%s\n",(char*)data->data);
    }
    fflush(stdout);
    data_destroy(data);
}

void del(struct rtable_t *rtable, char* key){
    int res = rtable_del(rtable,key);

    if (res != 0) {
        printf("Error in rtable_del or key not found!\n");
    } else {
        printf("Entry removed\n");
    }
    fflush(stdout);
}

void f_size(struct rtable_t *rtable){
    int res = rtable_size(rtable);
    if (res < 0)
        printf("Error in rtable_size!\n");
    else
        printf("Table size: %d\n",res);
    fflush(stdout);
}

void keys(struct rtable_t *rtable){
    char **key_list = rtable_get_keys(rtable);
    for (int i = 0; key_list[i] != NULL; i++) {
        printf("%s\n",key_list[i]);
    }
    rtable_free_keys(key_list);
}

void table(struct rtable_t *rtable){
    struct entry_t **entries = rtable_get_table(rtable);
    for (int j = 0; entries[j] != NULL; j++) {
        printf("%s :: %s\n",entries[j]->key,(char*)entries[j]->value->data);
    }
    rtable_free_entries(entries);
}

void stats(struct rtable_t *rtable){
    struct statistics_t *stats = rtable_stats(rtable);

    printf("Total operations executed: %d\n",stats->operations);
    printf("Time spent on operations: %ld us\n",stats->time_spent);
    printf("Clients currently connected to the server: %d\n",stats->clients);

    free(stats);
}

char** order_children(zoo_string *children_list) {
	char **list_copy = malloc((children_list->count+1)*sizeof(char*));
	char *temp = malloc(sizeof(char*));

	// SELECTION SORT IN PLACE
	for(int i = 0; i < children_list->count; i++){
		for(int j = i+1; j < children_list->count; j++){
            // Comparar os nodes para ver qual tem o id maior
			if(strcmp(children_list->data[i],children_list->data[j]) > 0){
				strcpy(temp,children_list->data[i]);
				strcpy(children_list->data[i],children_list->data[j]);
				strcpy(children_list->data[j],temp);
			}
		}
	}
    // Adicionar o prefixo /chain/ para completar o path de cada child
	for(int i = 0; i < children_list->count; i++){
		char *concat = malloc(strlen(children_list->data[i])+strlen("/chain/")+1);
		strcpy(concat,"/chain/");
		strcat(concat,children_list->data[i]);

		list_copy[i] = malloc(1024*sizeof(char));
		strcpy(list_copy[i],concat);
        free(concat);
	}
    list_copy[children_list->count] = NULL;
	free(temp);
	return list_copy;
}

void free_children(zoo_string* children) {
    for (int i = 0; i < children->count; i++) {
        free(children->data[i]);
    }
    free(children);
}

//---------- ZOOKEEPER -----------------------------

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			is_connected = 1; 
		} else {
			is_connected = 0; 
		}
	}
}

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
	zoo_string* children = (zoo_string *) malloc(sizeof(zoo_string));
	int zoo_data_len = ZDATALEN;
    char *zoo_data = malloc(ZDATALEN * sizeof(char));

	if (state == ZOO_CONNECTED_STATE) {
		if (type == ZOO_CHILD_EVENT) {
	 	    // Get the updated children and reset the watch
 			if (ZOK != zoo_wget_children(zh, "/chain", child_watcher, watcher_ctx, children)) {
 				printf("Error setting watch at /chain!\n");
 			}
            // Update the head and tail servers   
            char **children_list = order_children(children);

            // HEAD           
            zoo_get(wzh, children_list[0], 0, zoo_data, &zoo_data_len, NULL);
            // Se for uma head diferente da que esta ligada faz update
            if(strcmp(zoo_data,head) != 0){
                strcpy(head,zoo_data);
                // Reconnect
                rtable_disconnect(rtable_head);
                rtable_head = rtable_connect(head);
                if(rtable_head == NULL)
                    exit(1);
            }
            // TAIL
            zoo_get(wzh, children_list[children->count-1], 0, zoo_data, &zoo_data_len, NULL);
            // Se for uma tail diferente da que esta ligada faz update
            if(strcmp(zoo_data,tail) != 0){
                strcpy(tail,zoo_data);
                // Reconnect
                rtable_disconnect(rtable_tail);
                rtable_tail = rtable_connect(tail);
                if(rtable_tail == NULL)
                    exit(1);
            }
            // PRINTS
            printf("Established connection to head: %s\n", head);
            printf("Established connection to tail: %s\n", tail);
            fflush(stdout);

            table_free_keys(children_list);
            free_children(children);
	    }
    }
}

//---------- MAIN ----------------------------------

int main(int argc, char *argv[]){

    /* ZOOKEEPER: */

    if (argc != 2) {
        printf("Invalid args!\n");
        printf("Usage: table-client <zookeeperip:port>\n");
        fflush(stdout);
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);

    // Ligar a Zookeeper
    zh = zookeeper_init(argv[1], connection_watcher, 2000, 0, NULL, 0);
	if (zh == NULL) {
		printf("Error connecting to ZooKeeper server!\n");
		exit(EXIT_FAILURE);
	}

    // Espera que fique connected
    while(!is_connected){}
    
    // Tenta set watch a /chain
    if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL)) {
		printf("Error retrieving servers\n");
		exit(EXIT_FAILURE);
	}

    // Sets watch a /chain
    zoo_string* children = (zoo_string *) malloc(sizeof(zoo_string));
	if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, "Chain Watcher", children)) {
		printf("Error setting watch at /chain!\n");
	}

    // Vai buscar os 2 servers - head and tail - a que se vai tentar ligar
    int zoo_data_len = ZDATALEN;
    char *zoo_data = malloc(ZDATALEN * sizeof(char));
    char **children_list = order_children(children);
    // HEAD
    zoo_get(zh, children_list[0], 0, zoo_data, &zoo_data_len, NULL);
    head = malloc(strlen(zoo_data)+1);
    strcpy(head,zoo_data);
    // Connect
    rtable_head = rtable_connect(zoo_data);
    if(rtable_head == NULL){
        exit(1);
    }
    // TAIL
    zoo_get(zh, children_list[children->count-1], 0, zoo_data, &zoo_data_len, NULL);
    tail = malloc(strlen(zoo_data)+1);
    strcpy(tail,zoo_data);
    // Connect
    rtable_tail = rtable_connect(zoo_data);
    if(rtable_tail == NULL){
        exit(1);
    }
    free(zoo_data);

    // PRINTS
    printf("Established connection to head: %s\n", head);
    printf("Established connection to tail: %s\n", tail);
    fflush(stdout);
    table_free_keys(children_list);
    free_children(children);

    /* INTERACAO CLIENTE-SERVIDOR: */

    // Para o getline
    char *buf = malloc(sizeof(char*));
    size_t size = sizeof(char*);

    do {
        // Ler da linha de comandos
        printf("Command: ");
        getline(&buf,&size,stdin);

        // Testar se o input não é vazio
        if(exists(buf)){
            // Retirar o parágrafo e ver se resta alguma coisa
            char *input = strtok(buf,"\n");
            if(exists(input)){
                // Ir buscar o elemento antes do primeiro espaço
                char *command = strtok(input," ");
                if(exists(command)){
                    char *key = strtok(NULL," ");
                    // Ver se a chave existe
                    if(exists(key)){
                        if(strcmp(command,"p") == 0 || strcmp(command,"put") == 0){
                            char *value = strtok(NULL," ");
                            if(exists(value)){
                                put(rtable_head,key,(void*)value);
                            } else {
                                printf("Invalid arguments. Usage: put <key> <value>\n");
                                fflush(stdout);
                            }
                            continue;
                        }
                        else if(strcmp(command,"g") == 0|| strcmp(command,"get") == 0){
                            get(rtable_tail,key);
                            continue;
                        }
                        else if(strcmp(command,"d") == 0 || strcmp(command,"del") == 0){
                            del(rtable_head,key);
                            continue;
                        }
                    } else {
                        if(strcmp(command,"g") == 0 || strcmp(command,"get") == 0){
                            printf("Invalid arguments. Usage: get <key>\n");
                            fflush(stdout);
                            continue;
                        }
                        else if(strcmp(command,"d") == 0 || strcmp(command,"del") == 0){
                            printf("Invalid arguments. Usage: del <key>\n");
                            fflush(stdout);
                            continue;
                        }
                        else if(strcmp(command,"p") == 0 || strcmp(command,"put") == 0){
                            printf("Invalid arguments. Usage: put <key> <value>\n");
                            fflush(stdout);
                            continue;
                        }
                        else if(strcmp(command,"s") == 0 || strcmp(command,"size") == 0){
                            f_size(rtable_tail);
                            continue;
                        }
                        else if(strcmp(command,"k") == 0 || strcmp(command,"getkeys") == 0){
                            keys(rtable_tail);
                            continue;
                        }
                        else if(strcmp(command,"t") == 0 || strcmp(command,"gettable") == 0){
                            table(rtable_tail);
                            continue;
                        }
                        else if(strcmp(command,"stats") == 0){
                            stats(rtable_tail);
                            continue;
                        }
                        else if(strcmp(command,"q") == 0 || strcmp(command,"quit") == 0){
                            break;
                        }
                    }  
                }
            }
        }
        printf("Invalid command.\n");
        printf("Usage: p[ut] <key> <value> | g[et] <key> | d[el] <key> | s[ize] | [get]k[eys] | [get]t[able] | stats | q[uit]\n");
        fflush(stdout);
           
    } while(1);

    free(buf);
    printf("Bye, bye!\n");
    rtable_disconnect(rtable_head);
    rtable_disconnect(rtable_tail);
}