#include <stdio.h>
#include <stdlib.h>

#include "idp.h"
#include "cenario.h"
#include "mapa.h"
#include "gerador.h"

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

    printf("Cenário %d: mapa %dx%d, %d estações P, %d pacotes\n\n",
           indice, cenario->largura_mapa, cenario->altura_mapa,
           cenario->num_estacoes, cenario->total_pacotes);

    /* geração sequencial: um pacote por vez, round-robin entre as
     * estações, até esgotar o total do cenário. Sem coletores drenando as
     * filas nesta etapa, SEM_ESPACO é possível se o total superar a
     * capacidade somada das filas — reportado abaixo, distinto de "fim". */
    ResultadoGeracao resultado;
    while ((resultado = gerador_gerar(&gerador, NULL)) == GERACAO_OK) {
    }

    imprimir_mapa(mapa);

    printf("\nPacotes aguardando coleta por estação:\n");
    for (int i = 0; i < cenario->num_estacoes; i++) {
        printf("  estação %d em (%d, %d): %d pacotes\n",
               i, estacoes[i].posicao.x, estacoes[i].posicao.y,
               estacoes[i].total);
    }
    printf("faltando gerar: %d\n", gerador_pacotes_restantes(&gerador));
    if (resultado == GERACAO_SEM_ESPACO) {
        printf("geração interrompida: filas de coleta cheias "
               "(coletores drenam as filas na Issue #7)\n");
    }

    mapa_destruir(mapa);
    return 0;
}
