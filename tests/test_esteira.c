#include <stdio.h>

#include "esteira.h"
#include "cenario.h"

static int falhas = 0;

static void verificar(bool condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "ok" : "FALHOU", descricao);
    if (!condicao) {
        falhas++;
    }
}

static void teste_inicializacao(void)
{
    printf("inicialização da esteira:\n");
    Esteira e;
    verificar(!esteira_inicializar(&e, 0), "recusa tamanho zero");
    verificar(!esteira_inicializar(&e, -3), "recusa tamanho negativo");
    verificar(!esteira_inicializar(&e, MAX_ESTEIRA + 1),
              "recusa tamanho acima de MAX_ESTEIRA");
    verificar(!esteira_inicializar(NULL, 4), "recusa esteira nula");

    const Cenario *c = cenario_obter(0);
    verificar(esteira_inicializar(&e, c->tamanho_esteira),
              "aceita o tamanho do cenário 0");
    verificar(e.tamanho == c->tamanho_esteira && esteira_total(&e) == 0,
              "esteira respeita o tamanho do cenário e começa vazia");
}

static void teste_insercao(void)
{
    printf("inserção na entrada (in):\n");
    Esteira e;
    esteira_inicializar(&e, 4);

    Pacote p = { 0, PACOTE_AGUARDANDO, 0 };
    verificar(!esteira_inserir(&e, NULL), "recusa pacote nulo");
    verificar(esteira_inserir(&e, &p), "insere na entrada livre");
    verificar(p.estado == PACOTE_NA_ESTEIRA, "pacote inserido vira NA_ESTEIRA");
    verificar(e.posicoes[0] == &p, "pacote fica na posição 0 (in)");

    Pacote q = { 1, PACOTE_AGUARDANDO, 0 };
    verificar(!esteira_inserir(&e, &q), "recusa inserir com a entrada ocupada");
    verificar(esteira_total(&e) == 1, "pacote recusado não entra");
}

static void teste_avanco(void)
{
    printf("avanço posição a posição:\n");
    Esteira e;
    esteira_inicializar(&e, 4);

    Pacote p = { 0, PACOTE_AGUARDANDO, 0 };
    esteira_inserir(&e, &p);

    verificar(esteira_avancar(&e) && e.posicoes[1] == &p,
              "pacote avança uma posição por tick");
    esteira_avancar(&e);
    esteira_avancar(&e);
    verificar(e.posicoes[3] == &p, "pacote chega em out após tamanho-1 ticks");
    verificar(!esteira_avancar(&e), "pacote em out não avança (espera retirada)");
    verificar(e.posicoes[3] == &p && esteira_total(&e) == 1,
              "pacote permanece em out");
}

static void teste_capacidade_por_posicao(void)
{
    printf("capacidade 1 por posição:\n");
    Esteira e;
    esteira_inicializar(&e, 3);

    Pacote a = { 0, PACOTE_AGUARDANDO, 0 };
    Pacote b = { 1, PACOTE_AGUARDANDO, 0 };
    Pacote c = { 2, PACOTE_AGUARDANDO, 0 };

    esteira_inserir(&e, &a);
    esteira_avancar(&e);
    esteira_inserir(&e, &b);
    esteira_avancar(&e);
    esteira_inserir(&e, &c);
    verificar(e.posicoes[0] == &c && e.posicoes[1] == &b && e.posicoes[2] == &a,
              "três pacotes ocupam três posições distintas");

    verificar(!esteira_avancar(&e), "esteira cheia não avança (out ocupada)");
    verificar(e.posicoes[0] == &c && e.posicoes[1] == &b && e.posicoes[2] == &a,
              "pacotes não se sobrepõem nem se perdem");

    verificar(esteira_retirar(&e) == &a, "retira o pacote de out");
    verificar(esteira_avancar(&e), "retirada libera o avanço dos demais");
    verificar(e.posicoes[1] == &c && e.posicoes[2] == &b && e.posicoes[0] == NULL,
              "cada pacote avançou exatamente uma posição");
}

static void teste_retirada(void)
{
    printf("retirada na saída (out):\n");
    Esteira e;
    esteira_inicializar(&e, 2);
    verificar(esteira_retirar(&e) == NULL, "retirar de out vazia retorna NULL");

    Pacote p = { 0, PACOTE_AGUARDANDO, 0 };
    esteira_inserir(&e, &p);
    esteira_avancar(&e);
    Pacote *retirado = esteira_retirar(&e);
    verificar(retirado == &p && esteira_total(&e) == 0,
              "retira o pacote e a esteira esvazia");
    verificar(retirado->estado == PACOTE_NA_ESTEIRA,
              "retirar não muda o estado (decisão fica com o entregador)");
}

static void teste_fluxo_completo(void)
{
    printf("fluxo completo in -> out:\n");
    Esteira e;
    const Cenario *c = cenario_obter(1);
    esteira_inicializar(&e, c->tamanho_esteira);

    Pacote pacotes[30];
    int inseridos = 0;
    int retirados = 0;
    int ordem_ok = 1;
    int ticks = 0;
    /* limite de 1000 ticks: se algo travar, o teste falha em vez de rodar
     * pra sempre */
    while (retirados < 30 && ticks < 1000) {
        if (inseridos < 30 && esteira_inserir(&e, &pacotes[inseridos])) {
            pacotes[inseridos].id = inseridos;
            inseridos++;
        }
        esteira_avancar(&e);
        Pacote *saiu = esteira_retirar(&e);
        if (saiu != NULL) {
            if (saiu->id != retirados) {
                ordem_ok = 0;
            }
            retirados++;
        }
        ticks++;
    }
    verificar(retirados == 30, "todos os pacotes atravessam a esteira");
    verificar(ordem_ok, "pacotes saem na ordem em que entraram");
    verificar(esteira_total(&e) == 0, "esteira termina vazia");
}

int main(void)
{
    teste_inicializacao();
    teste_insercao();
    teste_avanco();
    teste_capacidade_por_posicao();
    teste_retirada();
    teste_fluxo_completo();

    if (falhas > 0) {
        printf("\n%d teste(s) falharam\n", falhas);
        return 1;
    }
    printf("\ntodos os testes passaram\n");
    return 0;
}
