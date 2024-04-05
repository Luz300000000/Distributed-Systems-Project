/*
* -- Grupo 27 --
* Ines Luz 57552
* Matilde Marques 58164
* Marta Lourenco 58249
*/

#include "network_client.h"
#include "client_stub-private.h"
#include "message-private.h"
#include "inet.h"
#include <errno.h>

int network_connect(struct rtable_t *rtable){
    if(rtable == NULL)
        return -1;

    struct sockaddr_in server;

    if((rtable->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating TCP socket\n");
        return -1; 
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(rtable->server_port);

    if(inet_pton(AF_INET,rtable->server_address,&server.sin_addr) < 1){
        printf("Error in inet_pton\n");
        close(rtable->sockfd);
        return -1;
    }

    if(connect(rtable->sockfd,(struct sockaddr *)&server,sizeof(server)) < 0){
        printf("Error in connect\n");
        close(rtable->sockfd);
        return -1;
    }

    return 0;
}

MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg){    
    int sockdf = rtable->sockfd;

    short len_send = message_t__get_packed_size(msg);
    uint16_t len_to_server = htons(len_send);
    void *buf_send = malloc(len_send);

    message_t__pack(msg,buf_send);
    
    if((write_all(sockdf,(char*)&len_to_server,sizeof(short))) < 1)
        return NULL;

    if((write_all(sockdf,buf_send,len_send)) < 1)
        return NULL;

    uint16_t len_r;

    if((read_all(sockdf,(char*)&len_r,sizeof(short))) < 1){   
        return NULL; 
    }

    short len_receive = ntohs(len_r);
    void *buf_receive = malloc(len_receive);

    if((read_all(sockdf,buf_receive,len_receive)) < 1)
        return NULL;

    MessageT *received = message_t__unpack(NULL,len_receive,buf_receive);
        if(received == NULL){
            perror("Erro no unpack da resposta do server");
            return NULL;
        }

    free(buf_receive);
    free(buf_send);
    return received;
}

int network_close(struct rtable_t *rtable){
    if(rtable == NULL || rtable->sockfd == -1)
        return -1;

    close(rtable->sockfd);
    return 0;
}