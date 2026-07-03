#ifndef GERADOR_H
#define GERADOR_H

#include "idp.h"
#include "mapa.h"
#include "estacao.h"

/* Gerador sequencial de pacotes: cada chamada a gerador_gerar produz um
 * pacote na estação da vez. Vira thread própria na Issue #9. Os pacotes
 * vivem no pool fixo do gerador — nada de malloc por pacote. */
typedef struct {
    Estacao *estacoes;
    int num_estacoes;
    int total_pacotes;
    int gerados;
    int proxima_estacao;
    Pacote pacotes[MAX_PACOTES];
} Gerador;

/* Espalha as estações P do cenário na área de coleta (coluna x=1,
 * espaçadas na vertical), marca as células como CELULA_ESTACAO_P e zera
 * o estado do gerador. Retorna false se o cenário estoura MAX_ESTACOES,
 * MAX_PACOTES ou os limites do mapa. */
bool gerador_inicializar(Gerador *gerador, const Cenario *cenario,
                         Mapa *mapa, Estacao *estacoes);

/* Gera o próximo pacote na estação da vez (round-robin) e o deixa na
 * fila de coleta dela. Retorna NULL se o total do cenário já foi gerado
 * ou se a fila da estação está cheia. O ritmo de geração (intervalos)
 * fica pra Issue #9. */
Pacote *gerador_gerar(Gerador *gerador);

/* Quantos pacotes do cenário ainda faltam gerar. */
int gerador_pacotes_restantes(const Gerador *gerador);

#endif /* GERADOR_H */
