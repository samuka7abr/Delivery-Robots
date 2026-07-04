#include <stddef.h>

#include "esteira.h"

bool esteira_inicializar(Esteira *esteira, int tamanho)
{
    if (esteira == NULL || tamanho <= 0 || tamanho > MAX_ESTEIRA) {
        return false;
    }
    esteira->tamanho = tamanho;
    for (int i = 0; i < tamanho; i++) {
        esteira->posicoes[i] = NULL;
    }
    pthread_mutex_init(&esteira->mutex, NULL);
    pthread_cond_init(&esteira->cond, NULL);
    return true;
}

void esteira_destruir(Esteira *esteira)
{
    if (esteira == NULL) {
        return;
    }
    pthread_mutex_destroy(&esteira->mutex);
    pthread_cond_destroy(&esteira->cond);
}

bool esteira_inserir(Esteira *esteira, Pacote *pacote)
{
    if (esteira == NULL || pacote == NULL) {
        return false;
    }
    pthread_mutex_lock(&esteira->mutex);
    if (esteira->posicoes[0] != NULL) {
        pthread_mutex_unlock(&esteira->mutex);
        return false;
    }
    pacote->estado = PACOTE_NA_ESTEIRA;
    esteira->posicoes[0] = pacote;
    /* pode ter surgido pacote alcançável no out (esteira de tamanho 1) */
    pthread_cond_broadcast(&esteira->cond);
    pthread_mutex_unlock(&esteira->mutex);
    return true;
}

bool esteira_avancar(Esteira *esteira)
{
    if (esteira == NULL) {
        return false;
    }
    pthread_mutex_lock(&esteira->mutex);
    bool moveu = false;
    /* varre de out pra in: a célula liberada por um pacote no tick
     * ja pode ser ocupada pelo de tras, mas ngm anda 2 posições */
    for (int i = esteira->tamanho - 2; i >= 0; i--) {
        if (esteira->posicoes[i] != NULL && esteira->posicoes[i + 1] == NULL) {
            esteira->posicoes[i + 1] = esteira->posicoes[i];
            esteira->posicoes[i] = NULL;
            moveu = true;
        }
    }
    if (moveu) {
        /* in pode ter liberado (coletor esperando) e/ou out pode ter
         * enchido (entregador esperando) */
        pthread_cond_broadcast(&esteira->cond);
    }
    pthread_mutex_unlock(&esteira->mutex);
    return moveu;
}

Pacote *esteira_retirar(Esteira *esteira)
{
    if (esteira == NULL) {
        return NULL;
    }
    pthread_mutex_lock(&esteira->mutex);
    int out = esteira->tamanho - 1;
    Pacote *pacote = esteira->posicoes[out];
    esteira->posicoes[out] = NULL;
    if (pacote != NULL) {
        pthread_cond_broadcast(&esteira->cond);
    }
    pthread_mutex_unlock(&esteira->mutex);
    return pacote;
}

bool esteira_saida_ocupada(Esteira *esteira)
{
    if (esteira == NULL) {
        return false;
    }
    pthread_mutex_lock(&esteira->mutex);
    bool ocupada = esteira->posicoes[esteira->tamanho - 1] != NULL;
    pthread_mutex_unlock(&esteira->mutex);
    return ocupada;
}

int esteira_total(Esteira *esteira)
{
    if (esteira == NULL) {
        return 0;
    }
    pthread_mutex_lock(&esteira->mutex);
    int total = 0;
    for (int i = 0; i < esteira->tamanho; i++) {
        if (esteira->posicoes[i] != NULL) {
            total++;
        }
    }
    pthread_mutex_unlock(&esteira->mutex);
    return total;
}
