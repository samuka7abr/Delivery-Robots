#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "cenario.h"
#include "coletor.h"
#include "entregador.h"
#include "esteira.h"
#include "gerador.h"
#include "idp.h"
#include "interface.h"
#include "mapa.h"

#ifndef TICK_US
#define TICK_US 100000 /* 100 ms por unidade de tempo; -DTICK_US acelera em testes */
#endif

#define COR_RESET    "\033[0m"
#define COR_NEGRITO  "\033[1m"
#define COR_CIANO    "\033[1;36m"
#define COR_VERDE    "\033[1;32m"
#define COR_AMARELO  "\033[1;33m"
#define COR_VERMELHO "\033[1;31m"

static bool ler_indice_cenario(const char *texto, int *indice)
{
    if (texto == NULL || indice == NULL) {
        return false;
    }

    char *fim;
    long valor = strtol(texto, &fim, 10);
    while (*fim == ' ' || *fim == '\t' || *fim == '\n') {
        fim++;
    }
    if (fim == texto || *fim != '\0' || valor < 0 || valor >= CENARIO_TOTAL) {
        return false;
    }
    *indice = (int)valor;
    return true;
}

static int escolher_cenario(void)
{
    static const char *cores[CENARIO_TOTAL] = {
        COR_VERDE,
        COR_AMARELO,
        COR_VERMELHO,
    };
    printf(COR_CIANO "IDP - Centro de Distribuição\n" COR_RESET);
    printf(COR_NEGRITO "Escolha o cenário:\n\n" COR_RESET);
    for (int i = 0; i < CENARIO_TOTAL; i++) {
        const Cenario *c = cenario_obter(i);
        printf("%s%d - %dx%d | %d coletores | %d entregadores | "
               "%d P | %d D | esteira %d | %d pacotes%s\n",
               cores[i], i,
               c->largura_mapa, c->altura_mapa,
               c->num_robos_coletores, c->num_robos_entregadores,
               c->num_estacoes, c->num_pontos_despacho,
               c->tamanho_esteira, c->total_pacotes, COR_RESET);
    }

    char entrada[32];
    int indice;
    while (true) {
        printf(COR_CIANO "\nCenário: " COR_RESET);
        fflush(stdout);
        if (fgets(entrada, sizeof entrada, stdin) == NULL) {
            return -1;
        }
        if (ler_indice_cenario(entrada, &indice)) {
            return indice;
        }
        printf(COR_VERMELHO "Opção inválida. Digite 0, 1 ou 2.\n" COR_RESET);
    }
}

static double agora_seg(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
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

static void stats_variar_na_esteira(int delta)
{
    pthread_mutex_lock(&stats.mutex);
    stats.pacotes_na_esteira += delta;
    pthread_mutex_unlock(&stats.mutex);
}

static void stats_somar_entregue(void)
{
    pthread_mutex_lock(&stats.mutex);
    stats.pacotes_entregues++;
    pthread_mutex_unlock(&stats.mutex);
}

static int stats_atualizar_e_obter_entregues(double tempo)
{
    pthread_mutex_lock(&stats.mutex);
    stats.tempo_execucao_seg = tempo;
    int entregues = stats.pacotes_entregues;
    pthread_mutex_unlock(&stats.mutex);
    return entregues;
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
            stats_variar_na_esteira(+1);
        }
        usleep(TICK_US / 2);
    }
    return NULL;
}

typedef struct {
    Entregador *entregador;
    Mapa *mapa;
    Esteira *esteira;
} ArgsEntregador;

static void *thread_entregador(void *arg)
{
    ArgsEntregador *a = arg;
    while (!deve_encerrar()) {
        ResultadoEntrega r = entregador_passo(a->entregador, a->mapa, a->esteira);
        if (r == ENTREGA_RETIROU) {
            stats_variar_na_esteira(-1);
        } else if (r == ENTREGA_ENTREGOU) {
            stats_somar_entregue();
        }
        usleep(TICK_US / 2);
    }
    return NULL;
}

/* --- saída headless ----------------------------------------------------- */

static void imprimir_frame_texto(int indice, const Cenario *cenario, Mapa *mapa,
                                 const Robo *robos, int num_robos,
                                 Esteira *esteira, Estatisticas *estatisticas)
{
    char mapa_txt[4096];
    char esteira_txt[3 + MAX_ESTEIRA + 4 + 1];
    int aguardando;
    int na_esteira;
    int entregues;
    double tempo;

    pthread_mutex_lock(&estatisticas->mutex);
    aguardando = estatisticas->pacotes_aguardando;
    na_esteira = estatisticas->pacotes_na_esteira;
    entregues = estatisticas->pacotes_entregues;
    tempo = estatisticas->tempo_execucao_seg;
    pthread_mutex_unlock(&estatisticas->mutex);

    printf("Cenário %d - mapa %dx%d\n\n", indice,
           cenario->largura_mapa, cenario->altura_mapa);
    if (interface_compor_mapa(mapa, robos, num_robos,
                              mapa_txt, sizeof mapa_txt) > 0) {
        fputs(mapa_txt, stdout);
    }
    if (interface_compor_esteira(esteira, esteira_txt, sizeof esteira_txt) > 0) {
        printf("\nEsteira: %s\n", esteira_txt);
    }
    printf("\nPacotes aguardando coleta : %d\n", aguardando);
    printf("Pacotes na esteira        : %d\n", na_esteira);
    printf("Pacotes entregues         : %d\n", entregues);
    printf("Tempo de execucao (s)     : %.1f\n", tempo);
}

int main(int argc, char **argv)
{
    bool terminal_interativo = isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
    int indice;
    if (argc > 2) {
        fprintf(stderr, "uso: %s [0|1|2]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        if (!ler_indice_cenario(argv[1], &indice)) {
            fprintf(stderr, "cenário inválido: use um índice entre 0 e %d\n",
                    CENARIO_TOTAL - 1);
            return 1;
        }
    } else if (terminal_interativo) {
        indice = escolher_cenario();
        if (indice < 0) {
            fprintf(stderr, "\nseleção de cenário cancelada\n");
            return 1;
        }
    } else {
        indice = 0;
    }

    const Cenario *cenario = cenario_obter(indice);

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

    Posicao entrada = { cenario->largura_mapa / 4, cenario->altura_mapa / 2 };

    Robo robos[MAX_ROBOS];
    Coletor coletores[MAX_ROBOS];
    int num_coletores = cenario->num_robos_coletores;
    for (int i = 0; i < num_coletores; i++) {
        Posicao inicial = { 2, 1 + i };
        if (!robo_inicializar(&robos[i], i, ROBO_COLETOR, inicial, mapa)) {
            fprintf(stderr, "falha ao posicionar o coletor %d\n", i);
            esteira_destruir(&esteira);
            mapa_destruir(mapa);
            return 1;
        }
        coletor_inicializar(&coletores[i], &robos[i], entrada);
    }

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
            esteira_destruir(&esteira);
            mapa_destruir(mapa);
            return 1;
        }
        entregador_inicializar(&entregadores[i], &robos[idx], saida,
                               pontos[i % num_pontos].posicao);
    }
    int total_robos = num_coletores + num_entregadores;

    stats.pacotes_aguardando = 0;
    stats.pacotes_na_esteira = 0;
    stats.pacotes_entregues = 0;
    stats.tempo_execucao_seg = 0.0;
    pthread_mutex_init(&stats.mutex, NULL);

    pthread_t th_gerador;
    pthread_t th_esteira;
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

    ArgsEntregador args_entregadores[MAX_ROBOS];
    for (int i = 0; i < num_entregadores; i++) {
        int idx = num_coletores + i;
        args_entregadores[i] = (ArgsEntregador){ &entregadores[i], mapa, &esteira };
        pthread_create(&robos[idx].thread, NULL, thread_entregador,
                       &args_entregadores[i]);
    }

    bool modo_interativo = terminal_interativo;
    if (modo_interativo) {
        interface_iniciar();
    }

    double inicio = agora_seg();
    const int max_iteracoes = 6000;
    for (int it = 0; it < max_iteracoes; it++) {
        usleep(TICK_US);
        int entregues = stats_atualizar_e_obter_entregues(agora_seg() - inicio);

        if (modo_interativo) {
            interface_desenhar(mapa, robos, total_robos, &esteira, &stats);
            if (interface_tecla_sair()) {
                break;
            }
        }
        if (entregues >= cenario->total_pacotes) {
            break;
        }
    }
    sinalizar_encerramento();

    pthread_join(th_gerador, NULL);
    pthread_join(th_esteira, NULL);
    for (int i = 0; i < num_coletores; i++) {
        pthread_join(robos[i].thread, NULL);
    }
    for (int i = 0; i < num_entregadores; i++) {
        pthread_join(robos[num_coletores + i].thread, NULL);
    }

    int entregues = stats_atualizar_e_obter_entregues(agora_seg() - inicio);
    if (modo_interativo) {
        interface_desenhar(mapa, robos, total_robos, &esteira, &stats);
        interface_encerrar();
    } else {
        imprimir_frame_texto(indice, cenario, mapa, robos, total_robos,
                             &esteira, &stats);
    }

    if (entregues >= cenario->total_pacotes) {
        printf("\ntodos os %d pacotes foram entregues nos pontos D.\n",
               cenario->total_pacotes);
    } else {
        printf("\nfluxo incompleto: %d de %d pacotes entregues.\n",
               entregues, cenario->total_pacotes);
    }

    for (int i = 0; i < cenario->num_estacoes; i++) {
        estacao_destruir(&estacoes[i]);
    }
    esteira_destruir(&esteira);
    pthread_mutex_destroy(&stats.mutex);
    pthread_mutex_destroy(&mutex_encerrar);
    mapa_destruir(mapa);
    return 0;
}
