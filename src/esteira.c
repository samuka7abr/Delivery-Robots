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
    return true;
}

bool esteira_inserir(Esteira *esteira, Pacote *pacote)
{
    if (esteira == NULL || pacote == NULL || esteira->posicoes[0] != NULL) {
        return false;
    }
    pacote->estado = PACOTE_NA_ESTEIRA;
    esteira->posicoes[0] = pacote;
    return true;
}

bool esteira_avancar(Esteira *esteira)
{
    if (esteira == NULL) {
        return false;
    }
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
    return moveu;
}

Pacote *esteira_retirar(Esteira *esteira)
{
    if (esteira == NULL) {
        return NULL;
    }
    int out = esteira->tamanho - 1;
    Pacote *pacote = esteira->posicoes[out];
    esteira->posicoes[out] = NULL;
    return pacote;
}

int esteira_total(const Esteira *esteira)
{
    if (esteira == NULL) {
        return 0;
    }
    int total = 0;
    for (int i = 0; i < esteira->tamanho; i++) {
        if (esteira->posicoes[i] != NULL) {
            total++;
        }
    }
    return total;
}
