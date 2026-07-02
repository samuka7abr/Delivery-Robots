#include <stdlib.h>

#include "robo.h"

bool robo_inicializar(Robo *robo, int id, TipoRobo tipo, Posicao inicial, Mapa *mapa)
{
    if (!mapa_celula_livre(mapa, inicial)) {
        return false;
    }

    robo->id = id;
    robo->tipo = tipo;
    robo->posicao = inicial;
    robo->pacote_atual = NULL;

    mapa_ocupar(mapa, inicial);
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
    if (!mapa_celula_livre(mapa, destino)) {
        return false;
    }

    mapa_liberar(mapa, robo->posicao);
    mapa_ocupar(mapa, destino);
    robo->posicao = destino;
    return true;
}
