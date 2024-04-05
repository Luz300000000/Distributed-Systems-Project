/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include <pthread.h>
#include "network_server.h"
#include "network_server-private.h"
#include "table_skel.h"
#include "table_skel-private.h"
#include "message-private.h"
#include "inet.h"

int network_server_init(short port){
    int sockfd;
    struct sockaddr_in server;

    // criar socket TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Erro ao criar socket no network_server_init");
        return -1;
    }
    
    // preencher estrutura server p/ bind
    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Porta TCP
    server.sin_addr.s_addr = htonl(INADDR_ANY); // Todos os endereços na máquina

    // para poder reutilizar um endereço
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Erro no setsockopt no network_server_init");
        close(sockfd);
        exit(1);
    }

    // bind
    if(bind(sockfd,(struct sockaddr *)&server,sizeof(server)) < 0){
        perror("Erro ao fazer bind no network_server_init");
        close(sockfd);
        return -1;
    }

    // listen
    if(listen(sockfd,0) < 0){
        perror("Erro ao executar listen no network_server_init");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int network_main_loop(int listening_socket, struct table_t *table, struct statistics_t *stats, struct semaphores_t *sem){
    int connsockfd;
    struct sockaddr_in client;
    socklen_t size_client = sizeof(client);
    pthread_t server_tid;

    while(1){
        connsockfd = accept(listening_socket,(struct sockaddr *)&client,&size_client);
        if(connsockfd == -1)
            continue;

        struct procedure_args *p_args = malloc(sizeof(struct procedure_args));
        
        p_args->client_socket = connsockfd;
        p_args->table = table;
        p_args->stats = stats;
        p_args->sem = sem;
        
        pthread_create(&server_tid,NULL,&client_procedure,(void *) p_args);
    }
}

void *client_procedure(void *args){
    struct procedure_args *p_args = (struct procedure_args *) args;
    printf("Client connection established\n");

    enter_write(p_args->sem->sem_stats);
    p_args->stats->clients++;
    leave_write(p_args->sem->sem_stats);

    while(1){
        MessageT *msg = network_receive(p_args->client_socket);
        if(msg == NULL){
            printf("Client connection closed\n");
            close(p_args->client_socket);
            break;
        }

        invoke(msg,p_args->table,p_args->stats,p_args->sem);

        if(network_send(p_args->client_socket,msg) < 0){
            printf("Client connection closed\n");
            close(p_args->client_socket);
            break;
        }

        message_t__free_unpacked(msg,NULL);
    }

    enter_write(p_args->sem->sem_stats);
    p_args->stats->clients--;
    leave_write(p_args->sem->sem_stats);
    
    pthread_exit(NULL);
}

MessageT *network_receive(int client_socket){
    uint16_t len_s;
    if((read_all(client_socket,(char*)&len_s,sizeof(short))) < 1)
        return NULL;

    short len = ntohs(len_s);
    void *buf = malloc(len);
    if(((read_all(client_socket,buf,len))) < 1)
        return NULL;

    MessageT *msg = message_t__unpack(NULL,len,buf);
    if(msg == NULL){
        perror("Erro no unpack da mensagem do cliente em network_receive");
        return NULL;
    }

    free(buf);
    return msg;
}

int network_send(int client_socket, MessageT *msg){
    short len = message_t__get_packed_size(msg);
    uint16_t len_s = htons(len);
    void *buf = malloc(len);
    message_t__pack(msg,buf);

    if((write_all(client_socket,(char*)&len_s,sizeof(short))) < 1)
        return -1;

    if((write_all(client_socket,buf,len)) < 1)
        return -1;

    free(buf);
    return 0;
}

int network_server_close(int socket, struct statistics_t *stats, struct semaphores_t *sem){
    if(socket == -1)
        return -1;

    close(socket);
    free(stats);

    pthread_mutex_destroy(&sem->sem_table->mutex);
    pthread_mutex_destroy(&sem->sem_stats->mutex);
    pthread_cond_destroy(&sem->sem_table->cond);
    pthread_cond_destroy(&sem->sem_stats->cond);

    free(sem->sem_stats);
    free(sem->sem_table);
    free(sem);

    return 0;
}