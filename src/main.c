#include <stdio.h>

#include "idp.h"
#include "mapa.h"
#include "robo.h"

/* Render textual do mapa (a interface real é a Issue #10). Os robôs são
 * desenhados por cima das células do mapa a partir das suas posições atuais. */
static char celula_char(TipoCelula tipo)
{
    switch (tipo) {
        case CELULA_PAREDE:    return '#';
        case CELULA_ESTACAO_P: return 'P';
        case CELULA_PONTO_D:   return 'D';
        case CELULA_LIVRE:
        default:               return '.';
    }
}

static void imprimir_mapa(const Mapa *mapa, Robo *robos, int num_robos)
{
    for (int y = 0; y < mapa->altura; y++) {
        for (int x = 0; x < mapa->largura; x++) {
            char c = celula_char(mapa->celulas[y][x]);
            for (int r = 0; r < num_robos; r++) {
                if (robos[r].posicao.x == x && robos[r].posicao.y == y) {
                    c = (robos[r].tipo == ROBO_COLETOR) ? 'C' : 'E';
                    break;
                }
            }
            putchar(c);
        }
        putchar('\n');
    }
}

/* Tenta mover o robô e relata o resultado, deixando visível quando o
 * movimento é recusado por limite do mapa ou por overlap com outra entidade. */
static void passo(Robo *robo, Mapa *mapa, int dx, int dy, const char *descricao)
{
    bool moveu = robo_mover(robo, mapa, dx, dy);
    printf("robô %d: %-28s -> %s (%d, %d)\n",
           robo->id, descricao, moveu ? "moveu" : "RECUSADO",
           robo->posicao.x, robo->posicao.y);
}

int main(void)
{
    const int largura = 10;
    const int altura = 6;

    Mapa *mapa = mapa_criar(largura, altura);
    if (mapa == NULL) {
        fprintf(stderr, "falha ao criar o mapa\n");
        return 1;
    }

    /* paredes nas bordas */
    for (int x = 0; x < largura; x++) {
        mapa_definir_tipo(mapa, (Posicao){ x, 0 }, CELULA_PAREDE);
        mapa_definir_tipo(mapa, (Posicao){ x, altura - 1 }, CELULA_PAREDE);
    }
    for (int y = 0; y < altura; y++) {
        mapa_definir_tipo(mapa, (Posicao){ 0, y }, CELULA_PAREDE);
        mapa_definir_tipo(mapa, (Posicao){ largura - 1, y }, CELULA_PAREDE);
    }
    /* uma parede interna, uma estação P e um ponto D */
    mapa_definir_tipo(mapa, (Posicao){ 4, 2 }, CELULA_PAREDE);
    mapa_definir_tipo(mapa, (Posicao){ 4, 3 }, CELULA_PAREDE);
    mapa_definir_tipo(mapa, (Posicao){ 1, 1 }, CELULA_ESTACAO_P);
    mapa_definir_tipo(mapa, (Posicao){ largura - 2, altura - 2 }, CELULA_PONTO_D);

    Robo robos[2];
    robo_inicializar(&robos[0], 0, ROBO_COLETOR,   (Posicao){ 2, 2 }, mapa);
    robo_inicializar(&robos[1], 1, ROBO_ENTREGADOR, (Posicao){ 3, 2 }, mapa);

    printf("Estado inicial:\n");
    imprimir_mapa(mapa, robos, 2);

    printf("\nMovimentando o robô coletor (0):\n");
    passo(&robos[0], mapa,  0, -1, "sobe (célula livre)");
    passo(&robos[0], mapa,  0, -1, "sobe (parede da borda)");
    passo(&robos[0], mapa, -1,  0, "esquerda (estação P livre)");
    passo(&robos[0], mapa, -1,  0, "esquerda (parede da borda)");
    passo(&robos[0], mapa,  0,  1, "desce (célula livre)");
    passo(&robos[0], mapa,  1,  0, "direita rumo ao entregador");
    passo(&robos[0], mapa,  1,  0, "direita (overlap com robô 1)");

    printf("\nEstado final:\n");
    imprimir_mapa(mapa, robos, 2);

    mapa_destruir(mapa);
    return 0;
}
