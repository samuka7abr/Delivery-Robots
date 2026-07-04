#ifndef COLETOR_H
#define COLETOR_H

#include "idp.h"
#include "mapa.h"
#include "estacao.h"
#include "esteira.h"
#include "robo.h"

/* Robô coletor: retira pacotes das estações P e os leva até a entrada (in)
 * da esteira, carregando no máximo um pacote por vez. Roda em thread própria
 * (main.c); cada chamada a coletor_passo executa um único passo do ciclo
 * (escolher estação, andar uma célula, coletar ou inserir) */

typedef enum {
    COLETOR_OCIOSO,        /* sem pacote: procurando uma estação com pacote */
    COLETOR_INDO_ESTACAO,  /* deslocando-se até a estação escolhida */
    COLETOR_INDO_ESTEIRA   /* carregando o pacote até a entrada da esteira */
} EstadoColetor;

typedef struct {
    Robo *robo;
    EstadoColetor estado;
    int estacao_alvo;   /* índice da estação buscada (válido fora de OCIOSO) */
    Posicao entrada;    /* posição da entrada (in) da esteira no mapa */
} Coletor;

/* Desfecho de um passo do coletor. */
typedef enum {
    COLETA_OCIOSO,        /* nada a fazer: nenhuma estação tem pacote */
    COLETA_ANDANDO,       /* deu um passo em direção ao alvo (ou replanejou) */
    COLETA_COLETOU,       /* retirou um pacote da estação */
    COLETA_INSERIU,       /* colocou o pacote na entrada da esteira */
    COLETA_AGUARDANDO_IN, /* chegou na entrada, mas o in está ocupado */
    COLETA_BLOQUEADO      /* não conseguiu andar (caminho ocupado) */
} ResultadoColeta;

/* Prepara o coletor a partir de um robô já posicionado no mapa (via
 * robo_inicializar, tipo ROBO_COLETOR) e da posição da entrada da esteira. */
void coletor_inicializar(Coletor *coletor, Robo *robo, Posicao entrada);

/* Executa um passo do ciclo de coleta e retorna o desfecho. As estruturas
 * compartilhadas que o passo acessa (mapa, estações, esteira) são protegidas
 * pelos locks internos de cada módulo */
ResultadoColeta coletor_passo(Coletor *coletor, Mapa *mapa, Estacao *estacoes,
                              int num_estacoes, Esteira *esteira);

#endif /* COLETOR_H */
