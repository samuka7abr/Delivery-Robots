#ifndef GERADOR_H
#define GERADOR_H

#include "idp.h"
#include "mapa.h"
#include "estacao.h"

/* Gerador de pacotes: cada chamada a gerador_gerar
 * produz um pacote na estação da vez.
 * Roda em thread própria, com o ritmo controlado pelo laço da thread
 * (main.c) */
typedef struct {
    Estacao *estacoes;
    int num_estacoes;
    int total_pacotes;
    int gerados;
    int proxima_estacao;
    Pacote pacotes[MAX_PACOTES];
} Gerador;

/* Desfecho de uma tentativa de geração. Separa "acabou o cenário" (fim
 * definitivo) de "não há espaço agora" (transitório: as filas de coleta
 * estão cheias e vão drenar quando os coletores agirem - Issue #7). */
typedef enum {
    GERACAO_OK,         /* pacote gerado e enfileirado numa estação */
    GERACAO_CONCLUIDA,  /* o total de pacotes do cenário já foi gerado */
    GERACAO_SEM_ESPACO  /* todas as estações estão com a fila cheia */
} ResultadoGeracao;

/* Espalha as estações P do cenário na área de coleta (coluna x=1,
 * espaçadas na vertical), marca as células como CELULA_ESTACAO_P e zera
 * o estado do gerador. Retorna false se mapa/cenário forem nulos, se o
 * cenário estoura MAX_ESTACOES, MAX_PACOTES ou os limites do mapa. */
bool gerador_inicializar(Gerador *gerador, const Cenario *cenario,
                         Mapa *mapa, Estacao *estacoes);

/* Gera o próximo pacote seguindo round-robin entre as estações: começa na
 * estação da vez e, se a fila dela estiver cheia, tenta as seguintes até
 * dar uma volta completa. Em GERACAO_OK, *out (quando não-NULL) recebe o
 * pacote gerado; nos demais desfechos *out vira NULL.
 * o intervalo entre gerações é responsabilidade da thread que chama,
 * não da função */
ResultadoGeracao gerador_gerar(Gerador *gerador, Pacote **out);

/* Quantos pacotes do cenário ainda faltam gerar. */
int gerador_pacotes_restantes(const Gerador *gerador);

#endif /* GERADOR_H */
