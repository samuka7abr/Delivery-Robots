#include <stddef.h>

#include "coletor.h"

void coletor_inicializar(Coletor *coletor, Robo *robo, Posicao entrada)
{
    coletor->robo = robo;
    coletor->estado = COLETOR_OCIOSO;
    coletor->estacao_alvo = -1;
    coletor->entrada = entrada;
    coletor->base = robo->posicao;
}

static int distancia_manhattan(Posicao a, Posicao b)
{
    int dx = a.x > b.x ? a.x - b.x : b.x - a.x;
    int dy = a.y > b.y ? a.y - b.y : b.y - a.y;
    return dx + dy;
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
                /* sem estação com pacote: recua para a base para não bloquear a
                 * vizinhança do in aos coletores que ainda carregam pacote */
                if (distancia_manhattan(coletor->robo->posicao, coletor->base) == 0 ||
                    !robo_passo_em_direcao(coletor->robo, mapa, coletor->base)) {
                    return COLETA_OCIOSO;
                }
                return COLETA_ANDANDO;
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
            if (!robo_passo_em_direcao(coletor->robo, mapa, estacao->posicao)) {
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
            if (!robo_passo_em_direcao(coletor->robo, mapa, coletor->entrada)) {
                return COLETA_BLOQUEADO;
            }
            return COLETA_ANDANDO;
        }
    }
    return COLETA_OCIOSO;
}
