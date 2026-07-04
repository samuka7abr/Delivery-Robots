#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "idp.h"
#include "cenario.h"
#include "mapa.h"
#include "gerador.h"
#include "esteira.h"
#include "coletor.h"

#ifndef TICK_US
#define TICK_US 100000 /* 100 ms por unidade de tempo; -DTICK_US acelera em testes */
#endif

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

/* --- encerramento coordenado ------------------------------------------- */

static pthread_mutex_t mutex_encerrar = PTHREAD_MUTEX_INITIALIZER;
static bool encerrar = false;

static bool deve_encerrar(void)
{
    pthread_mutex_lock(&mutex_encerrar);
    bool fim = encerrar;
    pthread_mutex_unlock(&mutex_encerrar);
    return fim;
}

/* Hoje nenhuma thread dorme em cond var (coletores fazem polling via passo).
 * Quando o entregador (#8) dormir na cond da esteira esperando pacote no out,
 * este encerramento precisa ganhar um pthread_cond_broadcast na cond dela. */
static void sinalizar_encerramento(void)
{
    pthread_mutex_lock(&mutex_encerrar);
    encerrar = true;
    pthread_mutex_unlock(&mutex_encerrar);
}

/* --- estatísticas compartilhadas --------------------------------------- */

static Estatisticas stats;

static void stats_variar_aguardando(int delta)
{
    pthread_mutex_lock(&stats.mutex);
    stats.pacotes_aguardando += delta;
    pthread_mutex_unlock(&stats.mutex);
}

static void stats_somar_na_esteira(void)
{
    pthread_mutex_lock(&stats.mutex);
    stats.pacotes_na_esteira++;
    pthread_mutex_unlock(&stats.mutex);
}

/* --- threads das entidades --------------------------------------------- */

typedef struct {
    Gerador *gerador;
} ArgsGerador;

static void *thread_gerador(void *arg)
{
    ArgsGerador *a = arg;
    while (!deve_encerrar()) {
        ResultadoGeracao r = gerador_gerar(a->gerador, NULL);
        if (r == GERACAO_OK) {
            stats_variar_aguardando(+1);
        } else if (r == GERACAO_CONCLUIDA) {
            break;
        }
        /* GERACAO_SEM_ESPACO: espera os coletores drenarem e tenta de novo */
        usleep(TICK_US);
    }
    return NULL;
}

static void *thread_esteira(void *arg)
{
    Esteira *esteira = arg;
    while (!deve_encerrar()) {
        esteira_avancar(esteira);
        usleep(TICK_US);
    }
    return NULL;
}

typedef struct {
    Coletor *coletor;
    Mapa *mapa;
    Estacao *estacoes;
    int num_estacoes;
    Esteira *esteira;
} ArgsColetor;

static void *thread_coletor(void *arg)
{
    ArgsColetor *a = arg;
    while (!deve_encerrar()) {
        ResultadoColeta r = coletor_passo(a->coletor, a->mapa, a->estacoes,
                                          a->num_estacoes, a->esteira);
        if (r == COLETA_COLETOU) {
            stats_variar_aguardando(-1);
        } else if (r == COLETA_INSERIU) {
            stats_somar_na_esteira();
        }
        usleep(TICK_US / 2);
    }
    return NULL;
}

/* ------------------------------------------------------------------------ */

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
            esteira_destruir(&esteira);
            mapa_destruir(mapa);
            return 1;
        }
        coletor_inicializar(&coletores[i], &robos[i], entrada);
    }

    stats.pacotes_aguardando = 0;
    stats.pacotes_na_esteira = 0;
    stats.pacotes_entregues = 0;
    stats.tempo_execucao_seg = 0.0;
    pthread_mutex_init(&stats.mutex, NULL);

    printf("Cenário %d: mapa %dx%d, %d estações P, %d coletores, "
           "esteira %d, %d pacotes\n\n",
           indice, cenario->largura_mapa, cenario->altura_mapa,
           cenario->num_estacoes, num_coletores, cenario->tamanho_esteira,
           cenario->total_pacotes);

    pthread_t th_gerador, th_esteira;
    ArgsGerador args_gerador = { &gerador };
    pthread_create(&th_gerador, NULL, thread_gerador, &args_gerador);
    pthread_create(&th_esteira, NULL, thread_esteira, &esteira);

    ArgsColetor args_coletores[MAX_ROBOS];
    for (int i = 0; i < num_coletores; i++) {
        args_coletores[i] = (ArgsColetor){ &coletores[i], mapa, estacoes,
                                           cenario->num_estacoes, &esteira };
        pthread_create(&robos[i].thread, NULL, thread_coletor,
                       &args_coletores[i]);
    }

    /* Supervisão provisória até a #8 (entregadores): sem ninguém drenando o
     * out, o fluxo estanca quando a esteira enche. Encerra quando os
     * contadores ficam ~3s sem mudar, com teto de 60s. Com o entregador,
     * isto vira: encerrar quando entregues >= cenario->total_pacotes. */
    int aguardando_ant = -1;
    int na_esteira_ant = -1;
    int iteracoes_paradas = 0;
    const int max_iteracoes = 600;
    for (int it = 0; it < max_iteracoes; it++) {
        usleep(TICK_US);
        pthread_mutex_lock(&stats.mutex);
        int aguardando = stats.pacotes_aguardando;
        int na_esteira = stats.pacotes_na_esteira;
        pthread_mutex_unlock(&stats.mutex);

        if (aguardando == aguardando_ant && na_esteira == na_esteira_ant) {
            iteracoes_paradas++;
            if (iteracoes_paradas >= 30) {
                break;
            }
        } else {
            iteracoes_paradas = 0;
        }
        aguardando_ant = aguardando;
        na_esteira_ant = na_esteira;
    }
    sinalizar_encerramento();

    pthread_join(th_gerador, NULL);
    pthread_join(th_esteira, NULL);
    for (int i = 0; i < num_coletores; i++) {
        pthread_join(robos[i].thread, NULL);
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

    printf("\nApós o encerramento:\n");
    printf("  pacotes gerados:            %d\n",
           cenario->total_pacotes - gerador_pacotes_restantes(&gerador));
    printf("  inseridos na esteira:       %d\n", stats.pacotes_na_esteira);
    printf("  atualmente na esteira:      %d\n", esteira_total(&esteira));
    printf("  aguardando nas estações:    %d\n", aguardando);
    printf("  em transporte (coletores):  %d\n", carregando);

    if (esteira_total(&esteira) > 0 || carregando > 0) {
        printf("\nfluxo estanca sem os entregadores drenando o out (Issue #8).\n");
    }

    for (int i = 0; i < cenario->num_estacoes; i++) {
        estacao_destruir(&estacoes[i]);
    }
    esteira_destruir(&esteira);
    pthread_mutex_destroy(&stats.mutex);
    mapa_destruir(mapa);
    return 0;
}
