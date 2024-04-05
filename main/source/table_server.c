/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <pthread.h>
#include <time.h>
#include "inet.h"
#include "table_skel.h"
#include "network_server.h"
#include "sdmessage.pb-c.h"
#include "stats.h"
#include "signal.h"
#include "semaphores.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include <zookeeper/zookeeper.h>

#define ZDATALEN 1024 * 1024

typedef struct String_vector zoo_string; 

zhandle_t *zh;
int is_connected;

char *my_addr;
char *my_path;
char *next_server = NULL;

struct rtable_t *rtable_write = NULL;

//---------- FUNÇÕES AUXILIARES ----------------------------------

char** order_children(zoo_string *children_list) {
	char **list_copy = malloc((children_list->count+1)*sizeof(char*));
	char *temp = malloc(sizeof(char*));

	// SELECTION SORT IN PLACE
	for(int i = 0; i < children_list->count; i++){
		for(int j = i+1; j < children_list->count; j++){
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
	char *zoo_data = malloc(ZDATALEN * sizeof(char));
	int zoo_data_len = ZDATALEN;

	if (state == ZOO_CONNECTED_STATE){
		if (type == ZOO_CHILD_EVENT){
	 	   // Get the updated children and reset the watch
			if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, watcher_ctx, children)) {
				printf("Error setting watch at /chain!\n");
			}
			// Dar update do next_server
			char **children_list = order_children(children);
			for (int i = 0; i < children->count; i++) {

        		// Focar na child logo a seguir ao current server, que sera o que tem o id mais alto a seguir
        		if(strcmp(my_path,children_list[i]) == 0){
					// Se existir um servidor a seguir ao current
					if(i+1 < children->count){
						zoo_get(zh, children_list[i+1], 0, zoo_data, &zoo_data_len, NULL);

						// Se o next_server nao existia ou mudou fazer update
						if(next_server == NULL || strcmp(zoo_data,next_server) != 0){
							// Se nao existia inicializa-lo
							if(next_server == NULL)
								next_server = malloc(zoo_data_len);
							strcpy(next_server,zoo_data);

							// Reconnect
							if(rtable_write != NULL)
								rtable_disconnect(rtable_write);
                			rtable_write = rtable_connect(zoo_data);
                			if(rtable_write == NULL)
                    			exit(1);

							printf("Next server is %s\n",next_server);
							fflush(stdout);
						}
					} else {
						// Se nao existir um next_server
						next_server = NULL;
						if(rtable_write != NULL)
							rtable_disconnect(rtable_write);
						rtable_write = NULL;
					}
					table_free_keys(children_list);
					free(children);
					free(zoo_data);
					break;
				}
        	}
		}
	}
}

//---------- MAIN ----------------------------------

int main(int argc, char *argv[]) {

	/* SOCKET CONNECTION: */

    if (argc != 4) {
        printf("Invalid args!\n");
        printf("Usage: table_server <zookeeperip:port> <port> <n_lists>\n");
        fflush(stdout);
        exit(1);
    }

    int port = atoi(argv[2]);
    int n_lists = atoi(argv[3]);

	struct table_t *table = table_skel_init(n_lists);
    struct statistics_t *stats = table_skel_stats_init();
    struct semaphores_t *sem = table_skel_semaphores_init();

    if (port < 1023) {
        printf("Bad port number\n");
        fflush(stdout);
        exit(1);
    }
    if (table == NULL) {
        printf("Error initializing table\n");
        fflush(stdout);
        exit(1);
    }

    int sockfd = network_server_init(port);

    if (sockfd == -1) {
        exit(1);
    }
	
    signal(SIGPIPE, SIG_IGN);

	/* ZOOKEEPER CONNECTION: */

	// Ligar ao Zookeeper
    zh = zookeeper_init(argv[1], connection_watcher, 2000, 0, NULL, 0);
	if (zh == NULL) {
		printf("Error connecting to ZooKeeper server!\n");
		exit(EXIT_FAILURE);
	}

	// Espera que fique connected
	while(!is_connected){}		
	
    // Se for o primeiro servidor a ser ligado deve criar a /chain
	if (ZNONODE == zoo_exists(zh, "/chain", 0, NULL)) {
		if (ZOK == zoo_create( zh, "/chain", NULL, -1, & ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0)) {
			printf("/chain created!\n");
			fflush(stdout);
		} else {
			printf("Error Creating /chain!\n");
			exit(EXIT_FAILURE);
		} 
	}
    
	// Adicionar o current server ao Zookeeper
    int new_path_len = 1024;
	int zoo_data_len = ZDATALEN;
	my_addr = malloc(new_path_len);
	my_path = malloc(new_path_len);

	// Ir buscar o ip:porto
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;

    getifaddrs (&ifap);
	for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
		// Interessa-nos em especifico enp0s3
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name,"enp0s3") == 0) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            strcpy(my_addr,inet_ntoa(sa->sin_addr));
		}
	}
	freeifaddrs(ifap);
	freeifaddrs(ifa);

	// Construcao do endereco para guardar na child criada
	strcat(my_addr,":");
	strcat(my_addr,argv[2]);

	printf("My server address: %s\n",my_addr);
    fflush(stdout);

	// Adicionar a chain com data referente ao seu ip:porto
    if (ZOK != zoo_create(zh, "/chain/node", my_addr, strlen(my_addr)+1, & ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, my_path, new_path_len)) {
		printf("Error creating znode from path /chain/node!\n");
		exit(EXIT_FAILURE);
	}
	printf("Ephemeral Sequencial ZNode created! ZNode path: %s\n", my_path);
	fflush(stdout);
		
    // Set next_server e prev_server
	char *zoo_data = malloc(ZDATALEN * sizeof(char));
	zoo_string* children =	(zoo_string *) malloc(sizeof(zoo_string));
	
	// Set watch
	if (ZOK != zoo_wget_children(zh, "/chain", &child_watcher, "Child Watcher", children)) {
		printf("Error setting watch at /chain!\n");
	}
	char **children_list = order_children(children);
	for (int i = 0; i < children->count; i++) {

		// Encontrar a current node
        if(strcmp(my_path,children_list[i]) == 0){
			// Focar na child logo antes ao my_server, que sera o que tem o id mais baixo a seguir
			if(i-1 >= 0){
				zoo_get(zh, children_list[i-1], 0, zoo_data, &zoo_data_len, NULL);
				printf("Previous server is %s\n",zoo_data);
				fflush(stdout);

				// Connect temporario
                struct rtable_t *rtable_copy = rtable_connect(zoo_data);
                if(rtable_copy == NULL)
                    exit(1);
				// Vai buscar a table ao prev_server e faz put() um a um
				struct entry_t **entries = rtable_get_table(rtable_copy);
				for (int i = 0; entries[i] != NULL; i++){
					table_put(table,entries[i]->key,entries[i]->value);
					entry_destroy(entries[i]);
				}
				free(entries);
				rtable_disconnect(rtable_copy);
			}
			table_free_keys(children_list);
			free(children);
			break;
        }
	}
	free(zoo_data);

	printf("Server ready, waiting for connections\n");
	network_main_loop(sockfd,table,stats,sem);

	rtable_disconnect(rtable_write);
    network_server_close(sockfd,stats,sem);
}