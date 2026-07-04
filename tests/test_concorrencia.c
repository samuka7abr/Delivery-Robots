#include <pthread.h>
#include <stdio.h>

#include "estacao.h"

#define ITERACOES 100000

typedef struct {
    Estacao *estacao;
    Pacote *pacote;
    pthread_barrier_t *inicio;
} ArgsEstacao;

static void *alternar_fila(void *arg)
{
    ArgsEstacao *args = arg;
    pthread_barrier_wait(args->inicio);
    for (int i = 0; i < ITERACOES; i++) {
        estacao_enfileirar(args->estacao, args->pacote);
        estacao_desenfileirar(args->estacao);
    }
    return NULL;
}

static void *consultar_fila(void *arg)
{
    ArgsEstacao *args = arg;
    pthread_barrier_wait(args->inicio);
    for (int i = 0; i < ITERACOES; i++) {
        estacao_fila_vazia(args->estacao);
        estacao_fila_cheia(args->estacao);
    }
    return NULL;
}

int main(void)
{
    Estacao estacao;
    Pacote pacote = { 0, PACOTE_AGUARDANDO, 0 };
    pthread_barrier_t inicio;
    pthread_t escritor;
    pthread_t leitor;

    estacao_inicializar(&estacao, (Posicao){ 1, 1 });
    pthread_barrier_init(&inicio, NULL, 2);

    ArgsEstacao args = { &estacao, &pacote, &inicio };
    pthread_create(&escritor, NULL, alternar_fila, &args);
    pthread_create(&leitor, NULL, consultar_fila, &args);

    pthread_join(escritor, NULL);
    pthread_join(leitor, NULL);

    pthread_barrier_destroy(&inicio);
    estacao_destruir(&estacao);
    printf("consultas concorrentes da estação concluídas\n");
    return 0;
}
