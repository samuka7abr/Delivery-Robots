#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "idp.h"
#include "cenario.h"
#include "mapa.h"
#include "gerador.h"
#include "esteira.h"
#include "coletor.h"
#include "entregador.h"

/* Render textual provisório (a interface real é a Issue #10). */
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

static void imprimir_mapa(const Mapa *mapa)
{
    for (int y = 0; y < mapa->altura; y++) {
        for (int x = 0; x < mapa->largura; x++) {
            putchar(celula_char(mapa->celulas[y][x]));
        }
        putchar('\n');
    }
}

int main(int argc, char **argv)
{
    int indice = (argc > 1) ? atoi(argv[1]) : 0;
    const Cenario *cenario = cenario_obter(indice);
    if (cenario == NULL) {
        fprintf(stderr, "cenário inválido: use um índice entre 0 e %d\n",
                CENARIO_TOTAL - 1);
        return 1;
    }

    Mapa *mapa = mapa_criar(cenario->largura_mapa, cenario->altura_mapa);
    if (mapa == NULL) {
        fprintf(stderr, "falha ao criar o mapa\n");
        return 1;
    }

    Estacao estacoes[MAX_ESTACOES];
    Gerador gerador;
    if (!gerador_inicializar(&gerador, cenario, mapa, estacoes)) {
        fprintf(stderr, "falha ao inicializar o gerador\n");
        mapa_destruir(mapa);
        return 1;
    }

    Esteira esteira;
    if (!esteira_inicializar(&esteira, cenario->tamanho_esteira)) {
        fprintf(stderr, "falha ao inicializar a esteira\n");
        mapa_destruir(mapa);
        return 1;
    }

    /* fluxo espacial esquerda -> direita: coleta à esquerda (estações P em x=1),
     * entrada (in) da esteira no primeiro quarto e saída (out) no terceiro
     * quarto — bem separadas para coletores e entregadores não disputarem as
     * mesmas células. Os coletores partem da borda esquerda. */
    Posicao entrada = { cenario->largura_mapa / 4, cenario->altura_mapa / 2 };

    Robo robos[MAX_ROBOS];
    Coletor coletores[MAX_ROBOS];
    int num_coletores = cenario->num_robos_coletores;
    for (int i = 0; i < num_coletores; i++) {
        Posicao inicial = { 2, 1 + i };
        if (!robo_inicializar(&robos[i], i, ROBO_COLETOR, inicial, mapa)) {
            fprintf(stderr, "falha ao posicionar o coletor %d\n", i);
            mapa_destruir(mapa);
            return 1;
        }
        coletor_inicializar(&coletores[i], &robos[i], entrada);
    }

    /* lado da expedição: a saída (out) da esteira e os pontos de despacho D na
     * borda direita, espalhados na vertical. Cada entregador atende um ponto D
     * fixo (distribuídos em rodízio), partindo logo à direita do out. */
    Posicao saida = { 3 * cenario->largura_mapa / 4, cenario->altura_mapa / 2 };

    PontoDespacho pontos[MAX_PONTOS_D];
    int num_pontos = cenario->num_pontos_despacho;
    for (int i = 0; i < num_pontos; i++) {
        Posicao pd = {
            cenario->largura_mapa - 2,
            (i + 1) * cenario->altura_mapa / (num_pontos + 1),
        };
        pontos[i].posicao = pd;
        mapa_definir_tipo(mapa, pd, CELULA_PONTO_D);
    }

    Entregador entregadores[MAX_ROBOS];
    int num_entregadores = cenario->num_robos_entregadores;
    for (int i = 0; i < num_entregadores; i++) {
        int idx = num_coletores + i;
        Posicao inicial = { 3 * cenario->largura_mapa / 4 + 1, 1 + i };
        if (!robo_inicializar(&robos[idx], idx, ROBO_ENTREGADOR, inicial, mapa)) {
            fprintf(stderr, "falha ao posicionar o entregador %d\n", i);
            mapa_destruir(mapa);
            return 1;
        }
        entregador_inicializar(&entregadores[i], &robos[idx], saida,
                               pontos[i % num_pontos].posicao);
    }

    printf("Cenário %d: mapa %dx%d, %d estações P, %d coletores, %d entregadores, "
           "%d pontos D, esteira %d, %d pacotes\n\n",
           indice, cenario->largura_mapa, cenario->altura_mapa,
           cenario->num_estacoes, num_coletores, num_entregadores, num_pontos,
           cenario->tamanho_esteira, cenario->total_pacotes);

    /* laço sequencial: a cada tick gera um pacote, cada coletor e cada
     * entregador dá um passo do seu ciclo e a esteira avança. Encerra quando
     * todos os pacotes foram entregues nos pontos D — ou, por segurança, quando
     * não há mais progresso possível em nenhuma entidade. */
    int inseridos = 0;
    int entregues = 0;
    int ticks = 0;
    const int ticks_max = 20000;
    while (ticks < ticks_max) {
        bool progresso = false;

        if (gerador_gerar(&gerador, NULL) == GERACAO_OK) {
            progresso = true;
        }

        for (int i = 0; i < num_coletores; i++) {
            ResultadoColeta r = coletor_passo(&coletores[i], mapa, estacoes,
                                              cenario->num_estacoes, &esteira);
            if (r == COLETA_INSERIU) {
                inseridos++;
                progresso = true;
            } else if (r == COLETA_ANDANDO || r == COLETA_COLETOU) {
                progresso = true;
            }
        }

        for (int i = 0; i < num_entregadores; i++) {
            ResultadoEntrega r = entregador_passo(&entregadores[i], mapa, &esteira);
            if (r == ENTREGA_ENTREGOU) {
                entregues++;
                progresso = true;
            } else if (r == ENTREGA_ANDANDO || r == ENTREGA_RETIROU) {
                progresso = true;
            }
        }

        /* o avanço da esteira também é progresso: um pacote a caminho do out
         * (com todos ociosos) ainda vai render entregas, então não pode
         * disparar o encerramento por "sem progresso" cedo demais. */
        if (esteira_avancar(&esteira)) {
            progresso = true;
        }
        ticks++;

        if (entregues >= cenario->total_pacotes) {
            break;
        }
        if (!progresso) {
            break;
        }
    }

    imprimir_mapa(mapa);

    int carregando_col = 0;
    for (int i = 0; i < num_coletores; i++) {
        if (robos[i].pacote_atual != NULL) {
            carregando_col++;
        }
    }
    int carregando_ent = 0;
    for (int i = 0; i < num_entregadores; i++) {
        if (robos[num_coletores + i].pacote_atual != NULL) {
            carregando_ent++;
        }
    }
    int aguardando = 0;
    for (int i = 0; i < cenario->num_estacoes; i++) {
        aguardando += estacoes[i].total;
    }

    printf("\nApós %d ticks:\n", ticks);
    printf("  pacotes gerados:              %d\n",
           cenario->total_pacotes - gerador_pacotes_restantes(&gerador));
    printf("  inseridos na esteira:         %d\n", inseridos);
    printf("  entregues nos pontos D:       %d\n", entregues);
    printf("  atualmente na esteira:        %d\n", esteira_total(&esteira));
    printf("  aguardando nas estações:      %d\n", aguardando);
    printf("  em transporte (coletores):    %d\n", carregando_col);
    printf("  em transporte (entregadores): %d\n", carregando_ent);

    if (entregues >= cenario->total_pacotes) {
        printf("\ntodos os %d pacotes foram entregues nos pontos D.\n",
               cenario->total_pacotes);
    } else {
        printf("\nfluxo incompleto: %d de %d pacotes entregues em %d ticks.\n",
               entregues, cenario->total_pacotes, ticks);
    }

    mapa_destruir(mapa);
    return 0;
}
