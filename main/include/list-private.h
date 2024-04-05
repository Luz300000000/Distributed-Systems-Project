#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "entry.h"

struct node_t {
	struct entry_t *entry;
	struct node_t  *next;
};

struct list_t {
	int size;
	struct node_t *head;
};

/* Função auxiliar para a destruição de listas. Permite obter o próximo node a
 * analisar, libertando o espaço de memória ocupado pelo node anterior a este.
 * Retorna o node obtido ou NULL em caso de erro ou se este não existir.
*/
struct node_t *get_node(struct node_t *node);

/* Função auxiliar para adicionar uma entry à lista. Neste caso, já existe uma
 * entry igual (com a mesma chave) à passada como argumento e esta será substituída
 * pela nova entry. A memória ocupada pela entry antiga será libertada.
 * Retorna 1 se a entry foi substituída com sucesso ou -1 em caso de erro.
*/
int list_add_exists(struct list_t *list, struct entry_t *new_entry, struct entry_t *old_entry);

/* Função auxiliar para adicionar uma entry à lista. Neste caso, não existe ainda
 * qualquer entry igual (com a mesma chave) à passada como argumento. Esta será
 * colocada na lista, seguindo a sua ordem crescente.
 * Retorna 0 se a entry for adicionada com sucesso ou -1 em caso de erro.
*/
int list_add_new(struct list_t *list, struct entry_t *entry);

/* Função auxiliar para a remoção de uma entry com a key especificada. Após
 * encontrar a entry pretendida e removê-la da lista, liberta a memória
 * ocupada pela mesma.
 * Retorna 0 se a entry foi removida com sucesso ou -1 em caso de erro.
*/
int list_remove_exists(struct list_t *list, char *key);

#endif
