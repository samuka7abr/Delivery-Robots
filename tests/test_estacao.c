#include <stdio.h>

#include "estacao.h"
#include "gerador.h"
#include "mapa.h"
#include "cenario.h"

static int falhas = 0;

static void verificar(bool condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "ok" : "FALHOU", descricao);
    if (!condicao) {
        falhas++;
    }
}

static void teste_fila_fifo(void)
{
    printf("fila da estação:\n");
    Estacao e;
    estacao_inicializar(&e, (Posicao){ 1, 1 });
    verificar(estacao_fila_vazia(&e), "fila começa vazia");

    Pacote p[3] = {
        { 0, PACOTE_AGUARDANDO, 0 },
        { 1, PACOTE_AGUARDANDO, 0 },
        { 2, PACOTE_AGUARDANDO, 0 },
    };
    for (int i = 0; i < 3; i++) {
        estacao_enfileirar(&e, &p[i]);
    }

    Pacote *a = estacao_desenfileirar(&e);
    Pacote *b = estacao_desenfileirar(&e);
    Pacote *c = estacao_desenfileirar(&e);
    verificar(a->id == 0 && b->id == 1 && c->id == 2, "saída em ordem FIFO");
    verificar(estacao_fila_vazia(&e), "fila esvazia após retirar tudo");
    verificar(estacao_desenfileirar(&e) == NULL, "desenfileirar de fila vazia retorna NULL");

    Pacote lote[MAX_FILA_ESTACAO + 1];
    int aceitos = 0;
    for (int i = 0; i < MAX_FILA_ESTACAO + 1; i++) {
        if (estacao_enfileirar(&e, &lote[i])) {
            aceitos++;
        }
    }
    verificar(aceitos == MAX_FILA_ESTACAO, "fila aceita no máximo MAX_FILA_ESTACAO pacotes");
    verificar(estacao_fila_cheia(&e), "fila reporta cheia no limite");
}

static void teste_gerador(void)
{
    printf("gerador de pacotes:\n");
    const Cenario *cenario = cenario_obter(0);
    Mapa *mapa = mapa_criar(cenario->largura_mapa, cenario->altura_mapa);
    Estacao estacoes[MAX_ESTACOES];
    Gerador gerador;

    verificar(gerador_inicializar(&gerador, cenario, mapa, estacoes),
              "inicializa com o cenário 0");

    bool estacoes_no_mapa = true;
    for (int i = 0; i < cenario->num_estacoes; i++) {
        Posicao p = estacoes[i].posicao;
        if (mapa->celulas[p.y][p.x] != CELULA_ESTACAO_P) {
            estacoes_no_mapa = false;
        }
    }
    verificar(estacoes_no_mapa, "células das estações marcadas como P no mapa");

    Pacote *a, *b, *c;
    verificar(gerador_gerar(&gerador, &a) == GERACAO_OK &&
              gerador_gerar(&gerador, &b) == GERACAO_OK &&
              gerador_gerar(&gerador, &c) == GERACAO_OK,
              "primeiras gerações retornam GERACAO_OK");
    verificar(a->estacao_origem == 0 && b->estacao_origem == 1 &&
              c->estacao_origem == 0, "round-robin entre as estações");
    verificar(a->estado == PACOTE_AGUARDANDO, "pacote nasce aguardando coleta");
    verificar(gerador_pacotes_restantes(&gerador) == cenario->total_pacotes - 3,
              "contagem de restantes");

    int gerados = 3;
    ResultadoGeracao r;
    while ((r = gerador_gerar(&gerador, NULL)) == GERACAO_OK) {
        gerados++;
    }
    verificar(gerados == cenario->total_pacotes, "gera exatamente o total do cenário");
    verificar(r == GERACAO_CONCLUIDA, "para com GERACAO_CONCLUIDA ao esgotar o total");

    int na_fila = 0;
    for (int i = 0; i < cenario->num_estacoes; i++) {
        na_fila += estacoes[i].total;
    }
    verificar(na_fila == cenario->total_pacotes, "todos os pacotes ficam nas filas");

    mapa_destruir(mapa);
}

static void teste_gerador_rejeita_mapa_pequeno_demais(void)
{
    printf("gerador com mapa pequeno demais:\n");
    /* altura 2 pra 3 estações: a divisão inteira da fórmula de
     * posicionamento faz duas estações caírem na mesma célula */
    Cenario cenario = {
        .largura_mapa           = 5,
        .altura_mapa            = 2,
        .num_robos_coletores    = 1,
        .num_robos_entregadores = 1,
        .num_estacoes           = 3,
        .num_pontos_despacho    = 1,
        .tamanho_esteira        = 4,
        .total_pacotes          = 10,
    };
    Mapa *mapa = mapa_criar(cenario.largura_mapa, cenario.altura_mapa);
    Estacao estacoes[MAX_ESTACOES];
    Gerador gerador;

    verificar(!gerador_inicializar(&gerador, &cenario, mapa, estacoes),
              "recusa cenário onde duas estações colidiriam na mesma célula");

    mapa_destruir(mapa);
}

static void teste_gerador_distingue_fim_de_fila_cheia(void)
{
    printf("gerador distingue fim de fila cheia:\n");
    /* 1 estação, fila com capacidade MAX_FILA_ESTACAO, total maior que a
     * fila e sem coletores drenando: a geração deve parar com SEM_ESPACO
     * (transitório) e não com CONCLUIDA, deixando pacotes por gerar. */
    Cenario cenario = {
        .largura_mapa           = 10,
        .altura_mapa            = 10,
        .num_robos_coletores    = 1,
        .num_robos_entregadores = 1,
        .num_estacoes           = 1,
        .num_pontos_despacho    = 1,
        .tamanho_esteira        = 4,
        .total_pacotes          = MAX_FILA_ESTACAO + 8,
    };
    Mapa *mapa = mapa_criar(cenario.largura_mapa, cenario.altura_mapa);
    Estacao estacoes[MAX_ESTACOES];
    Gerador gerador;
    verificar(gerador_inicializar(&gerador, &cenario, mapa, estacoes),
              "inicializa cenário de 1 estação");

    int gerados = 0;
    ResultadoGeracao r;
    while ((r = gerador_gerar(&gerador, NULL)) == GERACAO_OK) {
        gerados++;
    }
    verificar(r == GERACAO_SEM_ESPACO, "para com SEM_ESPACO quando a fila enche");
    verificar(gerados == MAX_FILA_ESTACAO, "gera só o que cabe na fila");
    verificar(gerador_pacotes_restantes(&gerador) == 8,
              "restam pacotes por gerar (fim ≠ fila cheia)");

    mapa_destruir(mapa);
}

static void teste_gerador_pula_estacao_cheia(void)
{
    printf("gerador pula estação cheia (liveness):\n");
    /* 2 estações; lota a fila da estação 0 e força o round-robin a começar
     * nela. gerador_gerar deve colocar o pacote na estação 1 (que tem
     * espaço) em vez de desistir. */
    Cenario cenario = {
        .largura_mapa           = 10,
        .altura_mapa            = 10,
        .num_robos_coletores    = 1,
        .num_robos_entregadores = 1,
        .num_estacoes           = 2,
        .num_pontos_despacho    = 1,
        .tamanho_esteira        = 4,
        .total_pacotes          = MAX_PACOTES,
    };
    Mapa *mapa = mapa_criar(cenario.largura_mapa, cenario.altura_mapa);
    Estacao estacoes[MAX_ESTACOES];
    Gerador gerador;
    gerador_inicializar(&gerador, &cenario, mapa, estacoes);

    Pacote lixo;
    while (!estacao_fila_cheia(&estacoes[0])) {
        estacao_enfileirar(&estacoes[0], &lixo);
    }
    gerador.proxima_estacao = 0;

    Pacote *p = NULL;
    verificar(gerador_gerar(&gerador, &p) == GERACAO_OK,
              "gera mesmo com a estação da vez cheia");
    verificar(p != NULL && p->estacao_origem == 1,
              "coloca o pacote na estação 1 (a que tem espaço)");

    mapa_destruir(mapa);
}

int main(void)
{
    teste_fila_fifo();
    teste_gerador();
    teste_gerador_rejeita_mapa_pequeno_demais();
    teste_gerador_distingue_fim_de_fila_cheia();
    teste_gerador_pula_estacao_cheia();

    if (falhas > 0) {
        printf("\n%d teste(s) falharam\n", falhas);
        return 1;
    }
    printf("\ntodos os testes passaram\n");
    return 0;
}
