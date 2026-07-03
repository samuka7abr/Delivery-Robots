#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "idp.h"
#include "cenario.h"
#include "mapa.h"
#include "gerador.h"
#include "esteira.h"
#include "coletor.h"

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

    /* entrada (in) da esteira no centro do mapa; os coletores partem da borda
     * direita, coletam nas estações P (coluna x=1) e levam o pacote até aqui. */
    Posicao entrada = { cenario->largura_mapa / 2, cenario->altura_mapa / 2 };

    Robo robos[MAX_ROBOS];
    Coletor coletores[MAX_ROBOS];
    int num_coletores = cenario->num_robos_coletores;
    for (int i = 0; i < num_coletores; i++) {
        Posicao inicial = { cenario->largura_mapa - 2, 1 + i };
        if (!robo_inicializar(&robos[i], i, ROBO_COLETOR, inicial, mapa)) {
            fprintf(stderr, "falha ao posicionar o coletor %d\n", i);
            mapa_destruir(mapa);
            return 1;
        }
        coletor_inicializar(&coletores[i], &robos[i], entrada);
    }

    printf("Cenário %d: mapa %dx%d, %d estações P, %d coletores, "
           "esteira %d, %d pacotes\n\n",
           indice, cenario->largura_mapa, cenario->altura_mapa,
           cenario->num_estacoes, num_coletores, cenario->tamanho_esteira,
           cenario->total_pacotes);

    /* laço sequencial: a cada tick gera um pacote, cada coletor dá um passo do
     * seu ciclo e a esteira avança. Sem entregadores drenando o out (Issue #8),
     * o fluxo estanca quando a esteira enche — o laço encerra ao não haver mais
     * progresso (nada gerado e nenhum coletor andando/coletando/inserindo). */
    int inseridos = 0;
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

        esteira_avancar(&esteira);
        ticks++;

        if (!progresso) {
            break;
        }
    }

    imprimir_mapa(mapa);

    int carregando = 0;
    for (int i = 0; i < num_coletores; i++) {
        if (robos[i].pacote_atual != NULL) {
            carregando++;
        }
    }
    int aguardando = 0;
    for (int i = 0; i < cenario->num_estacoes; i++) {
        aguardando += estacoes[i].total;
    }

    printf("\nApós %d ticks:\n", ticks);
    printf("  pacotes gerados:            %d\n",
           cenario->total_pacotes - gerador_pacotes_restantes(&gerador));
    printf("  inseridos na esteira:       %d\n", inseridos);
    printf("  atualmente na esteira:      %d\n", esteira_total(&esteira));
    printf("  aguardando nas estações:    %d\n", aguardando);
    printf("  em transporte (coletores):  %d\n", carregando);

    if (esteira_total(&esteira) > 0 || carregando > 0) {
        printf("\nfluxo estanca sem os entregadores drenando o out (Issue #8).\n");
    }

    mapa_destruir(mapa);
    return 0;
}
