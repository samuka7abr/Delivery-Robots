#include <stddef.h>

#include "coletor.h"

void coletor_inicializar(Coletor *coletor, Robo *robo, Posicao entrada)
{
    coletor->robo = robo;
    coletor->estado = COLETOR_OCIOSO;
    coletor->estacao_alvo = -1;
    coletor->entrada = entrada;
}

static int distancia_manhattan(Posicao a, Posicao b)
{
    int dx = a.x > b.x ? a.x - b.x : b.x - a.x;
    int dy = a.y > b.y ? a.y - b.y : b.y - a.y;
    return dx + dy;
}

/* Um passo greedy em direção ao alvo: reduz primeiro o eixo mais distante e,
 * se ele estiver bloqueado, tenta o outro. Sem desvio de obstáculos — os
 * cenários atuais não têm paredes internas. Retorna true se o robô se moveu. */
static bool passo_em_direcao(Robo *robo, Mapa *mapa, Posicao alvo)
{
    int dx = alvo.x - robo->posicao.x;
    int dy = alvo.y - robo->posicao.y;
    int sx = (dx > 0) - (dx < 0);
    int sy = (dy > 0) - (dy < 0);
    int mx = dx < 0 ? -dx : dx;
    int my = dy < 0 ? -dy : dy;

    if (mx >= my && sx != 0) {
        if (robo_mover(robo, mapa, sx, 0)) return true;
        if (sy != 0 && robo_mover(robo, mapa, 0, sy)) return true;
    } else {
        if (sy != 0 && robo_mover(robo, mapa, 0, sy)) return true;
        if (sx != 0 && robo_mover(robo, mapa, sx, 0)) return true;
    }
    return false;
}

/* Primeira estação (em ordem) com pacote aguardando. -1 se todas vazias. */
static int escolher_estacao(Estacao *estacoes, int num_estacoes)
{
    for (int i = 0; i < num_estacoes; i++) {
        if (!estacao_fila_vazia(&estacoes[i])) {
            return i;
        }
    }
    return -1;
}

ResultadoColeta coletor_passo(Coletor *coletor, Mapa *mapa, Estacao *estacoes,
                              int num_estacoes, Esteira *esteira)
{
    switch (coletor->estado) {
        case COLETOR_OCIOSO: {
            int alvo = escolher_estacao(estacoes, num_estacoes);
            if (alvo < 0) {
                return COLETA_OCIOSO;
            }
            coletor->estacao_alvo = alvo;
            coletor->estado = COLETOR_INDO_ESTACAO;
            return COLETA_ANDANDO;
        }
        case COLETOR_INDO_ESTACAO: {
            Estacao *estacao = &estacoes[coletor->estacao_alvo];
            if (distancia_manhattan(coletor->robo->posicao, estacao->posicao) <= 1) {
                Pacote *pacote = estacao_desenfileirar(estacao);
                if (pacote == NULL) {
                    /* estação esvaziou antes da chegada: replaneja */
                    coletor->estado = COLETOR_OCIOSO;
                    return COLETA_ANDANDO;
                }
                pacote->estado = PACOTE_TRANSPORTADO;
                coletor->robo->pacote_atual = pacote;
                coletor->estado = COLETOR_INDO_ESTEIRA;
                return COLETA_COLETOU;
            }
            if (!passo_em_direcao(coletor->robo, mapa, estacao->posicao)) {
                return COLETA_BLOQUEADO;
            }
            return COLETA_ANDANDO;
        }
        case COLETOR_INDO_ESTEIRA: {
            if (distancia_manhattan(coletor->robo->posicao, coletor->entrada) <= 1) {
                if (!esteira_inserir(esteira, coletor->robo->pacote_atual)) {
                    return COLETA_AGUARDANDO_IN;  /* in ocupado: aguarda */
                }
                coletor->robo->pacote_atual = NULL;
                coletor->estado = COLETOR_OCIOSO;
                return COLETA_INSERIU;
            }
            if (!passo_em_direcao(coletor->robo, mapa, coletor->entrada)) {
                return COLETA_BLOQUEADO;
            }
            return COLETA_ANDANDO;
        }
    }
    return COLETA_OCIOSO;
}
