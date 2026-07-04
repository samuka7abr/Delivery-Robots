#include <stdlib.h>

#include "robo.h"

bool robo_inicializar(Robo *robo, int id, TipoRobo tipo, Posicao inicial, Mapa *mapa)
{
    if (robo == NULL || !mapa_tentar_ocupar(mapa, inicial)) {
        return false;
    }

    robo->id = id;
    robo->tipo = tipo;
    robo->posicao = inicial;
    robo->pacote_atual = NULL;
    return true;
}

bool robo_mover(Robo *robo, Mapa *mapa, int dx, int dy)
{
    /* movimento é célula a célula, um eixo por vez: sem diagonal */
    bool passo_valido = (dx == 0 && (dy == 1 || dy == -1)) ||
                         (dy == 0 && (dx == 1 || dx == -1));
    if (!passo_valido) {
        return false;
    }

    Posicao destino = { robo->posicao.x + dx, robo->posicao.y + dy };
    if (!mapa_tentar_ocupar(mapa, destino)) {
        return false;
    }
    /* ocupa o destino antes de liberar a origem: o robô nunca fica sem
     * célula, então outra thread não toma seu lugar no meio do passo */
    mapa_liberar(mapa, robo->posicao);
    robo->posicao = destino;
    return true;
}
