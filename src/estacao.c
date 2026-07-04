#include <stddef.h>

#include "estacao.h"

void estacao_inicializar(Estacao *estacao, Posicao posicao)
{
    if (estacao == NULL) {
        return;
    }
    estacao->posicao = posicao;
    estacao->inicio = 0;
    estacao->fim = 0;
    estacao->total = 0;
    pthread_mutex_init(&estacao->mutex, NULL);
}

void estacao_destruir(Estacao *estacao)
{
    if (estacao == NULL) {
        return;
    }
    pthread_mutex_destroy(&estacao->mutex);
}

bool estacao_fila_vazia(Estacao *estacao)
{
    if (estacao == NULL) {
        return true;
    }
    pthread_mutex_lock(&estacao->mutex);
    bool vazia = estacao->total == 0;
    pthread_mutex_unlock(&estacao->mutex);
    return vazia;
}

bool estacao_fila_cheia(Estacao *estacao)
{
    if (estacao == NULL) {
        return false;
    }
    pthread_mutex_lock(&estacao->mutex);
    bool cheia = estacao->total == MAX_FILA_ESTACAO;
    pthread_mutex_unlock(&estacao->mutex);
    return cheia;
}

bool estacao_enfileirar(Estacao *estacao, Pacote *pacote)
{
    if (estacao == NULL || pacote == NULL) {
        return false;
    }
    pthread_mutex_lock(&estacao->mutex);
    if (estacao->total == MAX_FILA_ESTACAO) {
        pthread_mutex_unlock(&estacao->mutex);
        return false;
    }
    estacao->fila[estacao->fim] = pacote;
    estacao->fim = (estacao->fim + 1) % MAX_FILA_ESTACAO;
    estacao->total++;
    pthread_mutex_unlock(&estacao->mutex);
    return true;
}

Pacote *estacao_desenfileirar(Estacao *estacao)
{
    if (estacao == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&estacao->mutex);
    if (estacao->total == 0) {
        pthread_mutex_unlock(&estacao->mutex);
        return NULL;
    }
    Pacote *pacote = estacao->fila[estacao->inicio];
    estacao->inicio = (estacao->inicio + 1) % MAX_FILA_ESTACAO;
    estacao->total--;
    pthread_mutex_unlock(&estacao->mutex);
    return pacote;
}
