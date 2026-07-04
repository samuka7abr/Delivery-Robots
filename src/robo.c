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

bool robo_passo_em_direcao(Robo *robo, Mapa *mapa, Posicao alvo)
{
    int dx = alvo.x - robo->posicao.x;
    int dy = alvo.y - robo->posicao.y;
    int sx = (dx > 0) - (dx < 0);
    int sy = (dy > 0) - (dy < 0);
    int mx = dx < 0 ? -dx : dx;
    int my = dy < 0 ? -dy : dy;

    /* primeiro os passos que aproximam do alvo, reduzindo o eixo mais distante */
    if (mx >= my) {
        if (sx != 0 && robo_mover(robo, mapa, sx, 0)) return true;
        if (sy != 0 && robo_mover(robo, mapa, 0, sy)) return true;
    } else {
        if (sy != 0 && robo_mover(robo, mapa, 0, sy)) return true;
        if (sx != 0 && robo_mover(robo, mapa, sx, 0)) return true;
    }
    /* já alinhado num eixo e bloqueado no outro: contorna de lado para não
     * travar de frente com um robô vindo em sentido oposto no mesmo corredor */
    if (sx == 0 && (robo_mover(robo, mapa, 1, 0) || robo_mover(robo, mapa, -1, 0))) {
        return true;
    }
    if (sy == 0 && (robo_mover(robo, mapa, 0, 1) || robo_mover(robo, mapa, 0, -1))) {
        return true;
    }
    return false;
}
