#include <stdlib.h>

#include "mapa.h"

Mapa *mapa_criar(int largura, int altura)
{
    if (largura <= 0 || altura <= 0) {
        return NULL;
    }

    Mapa *mapa = malloc(sizeof(Mapa));
    if (mapa == NULL) {
        return NULL;
    }

    mapa->largura = largura;
    mapa->altura = altura;

    mapa->celulas = malloc(altura * sizeof(TipoCelula *));
    mapa->ocupada = malloc(altura * sizeof(bool *));
    if (mapa->celulas == NULL || mapa->ocupada == NULL) {
        free(mapa->celulas);
        free(mapa->ocupada);
        free(mapa);
        return NULL;
    }

    for (int y = 0; y < altura; y++) {
        mapa->celulas[y] = malloc(largura * sizeof(TipoCelula));
        mapa->ocupada[y] = malloc(largura * sizeof(bool));
        if (mapa->celulas[y] == NULL || mapa->ocupada[y] == NULL) {
            free(mapa->celulas[y]);
            free(mapa->ocupada[y]);
            for (int i = 0; i < y; i++) {
                free(mapa->celulas[i]);
                free(mapa->ocupada[i]);
            }
            free(mapa->celulas);
            free(mapa->ocupada);
            free(mapa);
            return NULL;
        }
        for (int x = 0; x < largura; x++) {
            mapa->celulas[y][x] = CELULA_LIVRE;
            mapa->ocupada[y][x] = false;
        }
    }

    /* mapa->mutex é inicializado ao introduzir as threads (Issue #9);
     * nesta etapa sequencial o mapa é acessado por uma única linha de execução. */
    return mapa;
}

void mapa_destruir(Mapa *mapa)
{
    if (mapa == NULL) {
        return;
    }

    for (int y = 0; y < mapa->altura; y++) {
        free(mapa->celulas[y]);
        free(mapa->ocupada[y]);
    }
    free(mapa->celulas);
    free(mapa->ocupada);

    free(mapa);
}

void mapa_definir_tipo(Mapa *mapa, Posicao p, TipoCelula tipo)
{
    if (!mapa_dentro_limites(mapa, p)) {
        return;
    }
    mapa->celulas[p.y][p.x] = tipo;
}

bool mapa_dentro_limites(const Mapa *mapa, Posicao p)
{
    return p.x >= 0 && p.x < mapa->largura &&
           p.y >= 0 && p.y < mapa->altura;
}

bool mapa_celula_livre(const Mapa *mapa, Posicao p)
{
    if (!mapa_dentro_limites(mapa, p)) {
        return false;
    }
    if (mapa->celulas[p.y][p.x] == CELULA_PAREDE) {
        return false;
    }
    return !mapa->ocupada[p.y][p.x];
}

void mapa_ocupar(Mapa *mapa, Posicao p)
{
    if (mapa_dentro_limites(mapa, p)) {
        mapa->ocupada[p.y][p.x] = true;
    }
}

void mapa_liberar(Mapa *mapa, Posicao p)
{
    if (mapa_dentro_limites(mapa, p)) {
        mapa->ocupada[p.y][p.x] = false;
    }
}
