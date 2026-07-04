#ifndef ENTREGADOR_H
#define ENTREGADOR_H

#include "idp.h"
#include "mapa.h"
#include "esteira.h"
#include "robo.h"

/* Robô entregador: retira pacotes da saída (out) da esteira e os leva até seu
 * ponto de despacho D, carregando no máximo um pacote por vez. É o espelho do
 * coletor no sentido inverso do fluxo (out -> D). Roda em thread própria
 * (main.c); cada chamada a entregador_passo executa um único passo do ciclo
 * (checar o out, andar uma célula, retirar ou entregar). */

typedef enum {
    ENTREGADOR_OCIOSO,       /* sem pacote: aguardando um pacote chegar no out */
    ENTREGADOR_INDO_ESTEIRA, /* deslocando-se até a saída (out) da esteira */
    ENTREGADOR_INDO_DESPACHO /* carregando o pacote até o ponto de despacho D */
} EstadoEntregador;

typedef struct {
    Robo *robo;
    EstadoEntregador estado;
    Posicao saida;     /* posição da saída (out) da esteira no mapa */
    Posicao despacho;  /* ponto de despacho D atendido por este entregador */
    Posicao base;      /* posição de espera; recua para cá quando sem trabalho */
} Entregador;

/* Desfecho de um passo do entregador. */
typedef enum {
    ENTREGA_OCIOSO,    /* nada a fazer: nenhum pacote na saída da esteira */
    ENTREGA_ANDANDO,   /* deu um passo em direção ao alvo (ou replanejou) */
    ENTREGA_RETIROU,   /* retirou um pacote da saída da esteira */
    ENTREGA_ENTREGOU,  /* entregou o pacote no ponto D (contabilizar despacho) */
    ENTREGA_BLOQUEADO  /* não conseguiu andar (caminho ocupado) */
} ResultadoEntrega;

/* Prepara o entregador a partir de um robô já posicionado no mapa (via
 * robo_inicializar, tipo ROBO_ENTREGADOR), da posição da saída (out) da esteira
 * e do ponto de despacho D que ele atende. */
void entregador_inicializar(Entregador *entregador, Robo *robo, Posicao saida,
                            Posicao despacho);

/* Executa um passo do ciclo de entrega e retorna o desfecho. Em ENTREGA_ENTREGOU
 * o pacote passa a PACOTE_ENTREGUE; a atualização do contador global de
 * despachados fica com o chamador (Estatisticas entra na #9). As estruturas
 * compartilhadas que o passo acessa (mapa, esteira) são protegidas pelos locks
 * internos de cada módulo. */
ResultadoEntrega entregador_passo(Entregador *entregador, Mapa *mapa,
                                  Esteira *esteira);

#endif /* ENTREGADOR_H */
