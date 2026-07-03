#ifndef IDP_H
#define IDP_H

#include <pthread.h>
#include <stdbool.h>

/* mapa tem um mutex geral pra checar/ocupar celula (Mapa.mutex).
 * cada estacao P tem seu proprio mutex, pra fila de pacotes.
 * esteira usa mutex + cond var, cobre avanco e as posicoes in/out.
 * ponto de despacho eh so uma celula do mapa, sem trava propria.
 * contadores (aguardando, na esteira, entregues, tempo) ficam num mutex so.
 * interface le tudo isso direto, sem trava nova. */

#define MAX_ROBOS        16
#define MAX_ESTACOES     8
#define MAX_PONTOS_D     8
#define MAX_ESTEIRA      32
#define MAX_FILA_ESTACAO 32
#define MAX_PACOTES      128

typedef struct {
    int x;
    int y;
} Posicao;

typedef enum {
    CELULA_LIVRE,
    CELULA_PAREDE,
    CELULA_ESTACAO_P,
    CELULA_PONTO_D
} TipoCelula;

/* Mapa: matriz de células (tipo fixo) + matriz de ocupação (mutável) */
typedef struct {
    int largura;
    int altura;
    TipoCelula **celulas;
    bool **ocupada;
    pthread_mutex_t mutex;
} Mapa;

typedef enum {
    PACOTE_AGUARDANDO,
    PACOTE_TRANSPORTADO,
    PACOTE_NA_ESTEIRA,
    PACOTE_ENTREGUE
} EstadoPacote;

typedef struct {
    int id;
    EstadoPacote estado;
    int estacao_origem;
} Pacote;

/* Estação geradora de pacotes (ponto P): fila FIFO limitada de pacotes
 * aguardando coleta. A capacidade é MAX_FILA_ESTACAO (backpressure: fila
 * cheia faz o gerador tentar outra estação ou esperar coleta). */
typedef struct {
    Posicao posicao;
    Pacote *fila[MAX_FILA_ESTACAO];
    int inicio;
    int fim;
    int total;
    pthread_mutex_t mutex;
} Estacao;

/* Esteira transportadora: posicoes[0] é a entrada (in), posicoes[tamanho-1] é a saída (out) */
typedef struct {
    int tamanho;
    Pacote *posicoes[MAX_ESTEIRA];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Esteira;

/* Ponto de despacho (ponto D); ocupação é controlada via Mapa.mutex */
typedef struct {
    Posicao posicao;
} PontoDespacho;

typedef enum {
    ROBO_COLETOR,
    ROBO_ENTREGADOR
} TipoRobo;

typedef struct {
    int id;
    TipoRobo tipo;
    Posicao posicao;
    Pacote *pacote_atual;
    pthread_t thread;
} Robo;

/* Estatísticas globais da simulação */
typedef struct {
    int pacotes_aguardando;
    int pacotes_na_esteira;
    int pacotes_entregues;
    double tempo_execucao_seg;
    pthread_mutex_t mutex;
} Estatisticas;

/* Cenário estático (parâmetros definidos na Issue #4) */
typedef struct {
    int largura_mapa;
    int altura_mapa;
    int num_robos_coletores;
    int num_robos_entregadores;
    int num_estacoes;
    int num_pontos_despacho;
    int tamanho_esteira;
    int total_pacotes;
} Cenario;

#endif /* IDP_H */
