#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "table.h"
#include "stats.h"
#include "inet.h"
#include "semaphores.h"
#include "sdmessage.pb-c.h"
#include "client_stub.h"
#include "network_client.h"

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists);

/* Liberta toda a memória ocupada pela tabela e todos os recursos 
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table);

/* Inicia as estatísticas.
 * Retorna a estrutura das estatísticas criada ou NULL em caso de erro.
 */
struct statistics_t *table_skel_stats_init();

/* Inicia os semáforos.
 * Retorna a estrutura dos semáforos criada ou NULL em caso de erro.
 */
struct semaphores_t *table_skel_semaphores_init();

/* Executa na tabela table a operação indicada pelo opcode contido em msg 
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int invoke(MessageT *msg, struct table_t *table, struct statistics_t *stats, struct semaphores_t *sem);
           
#endif
