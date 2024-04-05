#ifndef _NETWORK_SERVER_PRIVATE_H
#define _NETWORK_SERVER_PRIVATE_H

#include "network_server.h"

struct procedure_args{
   int client_socket;
   struct table_t *table;
   struct statistics_t *stats;
   struct semaphores_t *sem;
};

/* Define o procedimento de cada thread, à qual está associada um cliente.
   Lê e envia os pedidos enviados pelo cliente até este se desconectar do servidor.*/
void *client_procedure(void *args);

#endif