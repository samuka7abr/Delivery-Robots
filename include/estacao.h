#ifndef ESTACAO_H
#define ESTACAO_H

#include "idp.h"

/* Prepara a estação na posição dada, com a fila de coleta vazia.
 * O mutex da estação só é inicializado na Issue #9, com as threads. */
void estacao_inicializar(Estacao *estacao, Posicao posicao);

bool estacao_fila_vazia(const Estacao *estacao);
bool estacao_fila_cheia(const Estacao *estacao);

/* Coloca o pacote no fim da fila de coleta. Retorna false se a fila
 * está cheia. */
bool estacao_enfileirar(Estacao *estacao, Pacote *pacote);

/* Retira o pacote há mais tempo aguardando (FIFO). NULL se a fila
 * está vazia. */
Pacote *estacao_desenfileirar(Estacao *estacao);

#endif /* ESTACAO_H */
