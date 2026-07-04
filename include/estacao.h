#ifndef ESTACAO_H
#define ESTACAO_H

#include "idp.h"

/* Prepara a estação na posição dada, com a fila de coleta vazia.
 * e inicializa o mutex q protege a fila. */
void estacao_inicializar(Estacao *estacao, Posicao posicao);

void estacao_destruir(Estacao *estacao);
bool estacao_fila_vazia(Estacao *estacao);
bool estacao_fila_cheia(Estacao *estacao);

/* Coloca o pacote no fim da fila de coleta. Retorna false se a fila
 * está cheia. */
bool estacao_enfileirar(Estacao *estacao, Pacote *pacote);

/* Retira o pacote há mais tempo aguardando (FIFO). NULL se a fila
 * está vazia. */
Pacote *estacao_desenfileirar(Estacao *estacao);

#endif /* ESTACAO_H */
