#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

#include "idp.h"
#include "cenario.h"
#include "mapa.h"
#include "gerador.h"
#include "esteira.h"
#include "coletor.h"
#include "robo.h"
#include "interface.h"

static double agora_seg(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* Um tick sequencial: gera um pacote, cada coletor dá um passo do seu ciclo e a
 * esteira avança. Retorna true se houve algum progresso no tick. */
static bool simulacao_passo(Gerador *gerador, Coletor *coletores, int num_coletores,
                            Mapa *mapa, Estacao *estacoes, int num_estacoes,
                            Esteira *esteira)
{
    bool progresso = false;

    if (gerador_gerar(gerador, NULL) == GERACAO_OK) {
        progresso = true;
    }
    for (int i = 0; i < num_coletores; i++) {
        ResultadoColeta r = coletor_passo(&coletores[i], mapa, estacoes,
                                          num_estacoes, esteira);
        if (r == COLETA_INSERIU || r == COLETA_ANDANDO || r == COLETA_COLETOU) {
            progresso = true;
        }
    }
    esteira_avancar(esteira);
    return progresso;
}

static void coletar_estatisticas(Estatisticas *stats, const Estacao *estacoes,
                                 int num_estacoes, const Esteira *esteira,
                                 int entregues, double tempo)
{
    int aguardando = 0;
    for (int i = 0; i < num_estacoes; i++) {
        aguardando += estacoes[i].total;
    }
    stats->pacotes_aguardando = aguardando;
    stats->pacotes_na_esteira = esteira_total(esteira);
    stats->pacotes_entregues = entregues;
    stats->tempo_execucao_seg = tempo;
}

/* Um único frame em texto, com todos os elementos da interface. Usado quando
 * não há terminal interativo (ex.: testes/CI), onde o modo ncurses não roda. */
static void imprimir_frame_texto(int indice, const Cenario *cenario, const Mapa *mapa,
                                 const Robo *robos, int num_robos,
                                 const Esteira *esteira, const Estatisticas *stats)
{
    /* comporta o maior mapa dos cenários (40x20 -> 821 bytes com quebras) */
    char mapa_txt[4096];
    char belt[3 + MAX_ESTEIRA + 4 + 1];

    printf("Cenário %d — mapa %dx%d\n\n", indice,
           cenario->largura_mapa, cenario->altura_mapa);
    if (interface_compor_mapa(mapa, robos, num_robos, mapa_txt, sizeof mapa_txt) > 0) {
        fputs(mapa_txt, stdout);
    }
    if (interface_compor_esteira(esteira, belt, sizeof belt) > 0) {
        printf("\nEsteira: %s\n", belt);
    }
    printf("\nPacotes aguardando coleta : %d\n", stats->pacotes_aguardando);
    printf("Pacotes na esteira        : %d\n", stats->pacotes_na_esteira);
    printf("Pacotes entregues         : %d\n", stats->pacotes_entregues);
    printf("Tempo de execucao (s)     : %.1f\n", stats->tempo_execucao_seg);
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
    int total_robos = 0;
    for (int i = 0; i < num_coletores; i++) {
        Posicao inicial = { cenario->largura_mapa - 2, 1 + i };
        if (!robo_inicializar(&robos[total_robos], i, ROBO_COLETOR, inicial, mapa)) {
            fprintf(stderr, "falha ao posicionar o coletor %d\n", i);
            mapa_destruir(mapa);
            return 1;
        }
        coletor_inicializar(&coletores[i], &robos[total_robos], entrada);
        total_robos++;
    }

    /* Pontos de despacho e robôs entregadores são posicionados de forma
     * estática só para a interface exibir a área de expedição — o ciclo dos
     * entregadores é da Issue #8 e a integração com threads da #9. A interface
     * pode trabalhar com dados mockados nesta etapa (ver corpo da Issue #10). */
    int num_pontos = cenario->num_pontos_despacho;
    for (int i = 0; i < num_pontos; i++) {
        mapa_definir_tipo(mapa, (Posicao){ cenario->largura_mapa - 3, 2 + i },
                          CELULA_PONTO_D);
    }
    int num_entregadores = cenario->num_robos_entregadores;
    for (int i = 0; i < num_entregadores; i++) {
        Posicao inicial = { cenario->largura_mapa - 5, 2 + i };
        if (robo_inicializar(&robos[total_robos], num_coletores + i,
                             ROBO_ENTREGADOR, inicial, mapa)) {
            total_robos++;
        }
    }

    Estatisticas stats = {
        .pacotes_aguardando = 0,
        .pacotes_na_esteira = 0,
        .pacotes_entregues = 0,
        .tempo_execucao_seg = 0.0,
    };
    int entregues = 0;   /* mockado até os entregadores (#8) drenarem o out */
    double t0 = agora_seg();

    if (isatty(STDOUT_FILENO)) {
        interface_iniciar();
        bool ativo = true;
        while (true) {
            if (ativo && !simulacao_passo(&gerador, coletores, num_coletores, mapa,
                                          estacoes, cenario->num_estacoes, &esteira)) {
                ativo = false;   /* fluxo estanca sem os entregadores (#8) */
            }
            coletar_estatisticas(&stats, estacoes, cenario->num_estacoes,
                                 &esteira, entregues, agora_seg() - t0);
            interface_desenhar(mapa, robos, total_robos, &esteira, &stats);
            if (interface_tecla_sair()) {
                break;
            }
            usleep(80000);
        }
        interface_encerrar();
    } else {
        int ticks = 0;
        const int ticks_max = 20000;
        while (ticks < ticks_max &&
               simulacao_passo(&gerador, coletores, num_coletores, mapa, estacoes,
                               cenario->num_estacoes, &esteira)) {
            ticks++;
        }
        coletar_estatisticas(&stats, estacoes, cenario->num_estacoes, &esteira,
                             entregues, agora_seg() - t0);
        imprimir_frame_texto(indice, cenario, mapa, robos, total_robos,
                             &esteira, &stats);
    }

    mapa_destruir(mapa);
    return 0;
}
