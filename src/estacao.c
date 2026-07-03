#include <stddef.h>

#include "estacao.h"

void estacao_inicializar(Estacao *estacao, Posicao posicao)
{
    estacao->posicao = posicao;
    estacao->inicio = 0;
    estacao->fim = 0;
    estacao->total = 0;
}

bool estacao_fila_vazia(const Estacao *estacao)
{
    return estacao->total == 0;
}

bool estacao_fila_cheia(const Estacao *estacao)
{
    return estacao->total == MAX_FILA_ESTACAO;
}

bool estacao_enfileirar(Estacao *estacao, Pacote *pacote)
{
    if (estacao_fila_cheia(estacao)) {
        return false;
    }
    estacao->fila[estacao->fim] = pacote;
    estacao->fim = (estacao->fim + 1) % MAX_FILA_ESTACAO;
    estacao->total++;
    return true;
}

Pacote *estacao_desenfileirar(Estacao *estacao)
{
    if (estacao_fila_vazia(estacao)) {
        return NULL;
    }
    Pacote *pacote = estacao->fila[estacao->inicio];
    estacao->inicio = (estacao->inicio + 1) % MAX_FILA_ESTACAO;
    estacao->total--;
    return pacote;
}
