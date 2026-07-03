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

    Pacote lote[MAX_ESTEIRA + 1];
    int aceitos = 0;
    for (int i = 0; i < MAX_ESTEIRA + 1; i++) {
        if (estacao_enfileirar(&e, &lote[i])) {
            aceitos++;
        }
    }
    verificar(aceitos == MAX_ESTEIRA, "fila aceita no máximo MAX_ESTEIRA pacotes");
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

    Pacote *a = gerador_gerar(&gerador);
    Pacote *b = gerador_gerar(&gerador);
    Pacote *c = gerador_gerar(&gerador);
    verificar(a->estacao_origem == 0 && b->estacao_origem == 1 &&
              c->estacao_origem == 0, "round-robin entre as estações");
    verificar(a->estado == PACOTE_AGUARDANDO, "pacote nasce aguardando coleta");
    verificar(gerador_pacotes_restantes(&gerador) == cenario->total_pacotes - 3,
              "contagem de restantes");

    int gerados = 3;
    while (gerador_gerar(&gerador) != NULL) {
        gerados++;
    }
    verificar(gerados == cenario->total_pacotes, "gera exatamente o total do cenário");

    int na_fila = 0;
    for (int i = 0; i < cenario->num_estacoes; i++) {
        na_fila += estacoes[i].total;
    }
    verificar(na_fila == cenario->total_pacotes, "todos os pacotes ficam nas filas");

    mapa_destruir(mapa);
}

int main(void)
{
    teste_fila_fifo();
    teste_gerador();

    if (falhas > 0) {
        printf("\n%d teste(s) falharam\n", falhas);
        return 1;
    }
    printf("\ntodos os testes passaram\n");
    return 0;
}
