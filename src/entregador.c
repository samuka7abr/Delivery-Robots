#include <stddef.h>

#include "entregador.h"

void entregador_inicializar(Entregador *entregador, Robo *robo, Posicao saida,
                            Posicao despacho)
{
    entregador->robo = robo;
    entregador->estado = ENTREGADOR_OCIOSO;
    entregador->saida = saida;
    entregador->despacho = despacho;
    entregador->base = robo->posicao;
}

static int distancia_manhattan(Posicao a, Posicao b)
{
    int dx = a.x > b.x ? a.x - b.x : b.x - a.x;
    int dy = a.y > b.y ? a.y - b.y : b.y - a.y;
    return dx + dy;
}

ResultadoEntrega entregador_passo(Entregador *entregador, Mapa *mapa,
                                  Esteira *esteira)
{
    switch (entregador->estado) {
        case ENTREGADOR_OCIOSO: {
            if (!esteira_saida_ocupada(esteira)) {
                /* out vazio: recua para a base para não bloquear sua vizinhança
                 * aos entregadores que ainda carregam pacote */
                if (distancia_manhattan(entregador->robo->posicao, entregador->base) == 0 ||
                    !robo_passo_em_direcao(entregador->robo, mapa, entregador->base)) {
                    return ENTREGA_OCIOSO;
                }
                return ENTREGA_ANDANDO;
            }
            entregador->estado = ENTREGADOR_INDO_ESTEIRA;
            return ENTREGA_ANDANDO;
        }
        case ENTREGADOR_INDO_ESTEIRA: {
            if (distancia_manhattan(entregador->robo->posicao, entregador->saida) <= 1) {
                Pacote *pacote = esteira_retirar(esteira);
                if (pacote == NULL) {
                    /* out esvaziou antes da chegada (outro entregador levou):
                     * replaneja */
                    entregador->estado = ENTREGADOR_OCIOSO;
                    return ENTREGA_ANDANDO;
                }
                pacote->estado = PACOTE_TRANSPORTADO;
                entregador->robo->pacote_atual = pacote;
                entregador->estado = ENTREGADOR_INDO_DESPACHO;
                return ENTREGA_RETIROU;
            }
            if (!robo_passo_em_direcao(entregador->robo, mapa, entregador->saida)) {
                return ENTREGA_BLOQUEADO;
            }
            return ENTREGA_ANDANDO;
        }
        case ENTREGADOR_INDO_DESPACHO: {
            if (distancia_manhattan(entregador->robo->posicao, entregador->despacho) <= 1) {
                entregador->robo->pacote_atual->estado = PACOTE_ENTREGUE;
                entregador->robo->pacote_atual = NULL;
                entregador->estado = ENTREGADOR_OCIOSO;
                return ENTREGA_ENTREGOU;
            }
            if (!robo_passo_em_direcao(entregador->robo, mapa, entregador->despacho)) {
                return ENTREGA_BLOQUEADO;
            }
            return ENTREGA_ANDANDO;
        }
    }
    return ENTREGA_OCIOSO;
}
