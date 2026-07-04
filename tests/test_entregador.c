#include <stdio.h>

#include "entregador.h"

static int falhas = 0;

static void verificar(bool condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "ok" : "FALHOU", descricao);
    if (!condicao) {
        falhas++;
    }
}

/* Roda passos até o desfecho alvo (ou o limite). Retorna o último resultado. */
static ResultadoEntrega rodar_ate(Entregador *e, Mapa *m, Esteira *es,
                                  ResultadoEntrega alvo, int max)
{
    ResultadoEntrega r = ENTREGA_OCIOSO;
    for (int i = 0; i < max; i++) {
        r = entregador_passo(e, m, es);
        if (r == alvo) {
            return r;
        }
    }
    return r;
}

/* Coloca um pacote diretamente na saída (out) da esteira. */
static void colocar_no_out(Esteira *es, Pacote *p)
{
    p->estado = PACOTE_NA_ESTEIRA;
    es->posicoes[es->tamanho - 1] = p;
}

static void teste_inicializacao(void)
{
    printf("inicialização do entregador:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_ENTREGADOR, (Posicao){ 6, 2 }, m);

    Entregador e;
    entregador_inicializar(&e, &robo, (Posicao){ 4, 2 }, (Posicao){ 1, 2 });
    verificar(e.estado == ENTREGADOR_OCIOSO, "entregador começa ocioso");
    verificar(e.robo == &robo && e.robo->pacote_atual == NULL,
              "entregador começa sem pacote");

    mapa_destruir(m);
}

static void teste_sem_pacote(void)
{
    printf("nenhum pacote na saída da esteira:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_ENTREGADOR, (Posicao){ 6, 2 }, m);
    Entregador e;
    entregador_inicializar(&e, &robo, (Posicao){ 4, 2 }, (Posicao){ 1, 2 });

    Esteira es;
    esteira_inicializar(&es, 4);

    verificar(entregador_passo(&e, m, &es) == ENTREGA_OCIOSO,
              "sem pacote no out o entregador fica ocioso");
    verificar(e.estado == ENTREGADOR_OCIOSO, "estado permanece ocioso");

    mapa_destruir(m);
}

static void teste_ciclo_completo(void)
{
    printf("ciclo completo out -> ponto D:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_ENTREGADOR, (Posicao){ 6, 2 }, m);
    Posicao saida = { 4, 2 };
    Posicao despacho = { 1, 2 };
    mapa_definir_tipo(m, despacho, CELULA_PONTO_D);
    Entregador e;
    entregador_inicializar(&e, &robo, saida, despacho);

    Esteira es;
    esteira_inicializar(&es, 4);
    Pacote p = { 7, PACOTE_NA_ESTEIRA, 0 };
    colocar_no_out(&es, &p);

    verificar(rodar_ate(&e, m, &es, ENTREGA_RETIROU, 100) == ENTREGA_RETIROU,
              "entregador alcança a saída e retira o pacote");
    verificar(robo.pacote_atual == &p && p.estado == PACOTE_TRANSPORTADO,
              "pacote fica em transporte com o robô");
    verificar(!esteira_saida_ocupada(&es), "pacote sai da saída da esteira");

    verificar(rodar_ate(&e, m, &es, ENTREGA_ENTREGOU, 100) == ENTREGA_ENTREGOU,
              "entregador leva o pacote até o ponto D e entrega");
    verificar(p.estado == PACOTE_ENTREGUE, "pacote fica marcado como entregue");
    verificar(robo.pacote_atual == NULL && e.estado == ENTREGADOR_OCIOSO,
              "entregador entrega e volta a ficar ocioso");

    mapa_destruir(m);
}

static void teste_um_pacote_por_vez(void)
{
    printf("no máximo um pacote por vez:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_ENTREGADOR, (Posicao){ 6, 2 }, m);
    Posicao saida = { 4, 2 };
    Posicao despacho = { 1, 2 };
    mapa_definir_tipo(m, despacho, CELULA_PONTO_D);
    Entregador e;
    entregador_inicializar(&e, &robo, saida, despacho);

    Esteira es;
    esteira_inicializar(&es, 4);
    Pacote a = { 0, PACOTE_NA_ESTEIRA, 0 };
    Pacote b = { 1, PACOTE_NA_ESTEIRA, 0 };
    colocar_no_out(&es, &a);

    rodar_ate(&e, m, &es, ENTREGA_RETIROU, 100);
    verificar(robo.pacote_atual == &a, "retira o pacote que estava no out");

    /* enquanto carrega o primeiro, um novo pacote surge no out; o entregador não
     * pode retirá-lo antes de entregar o que carrega - invariante checado a cada
     * passo, sem depender de contagem fixa */
    colocar_no_out(&es, &b);
    bool violou = false;
    ResultadoEntrega r = ENTREGA_OCIOSO;
    for (int i = 0; i < 100 && r != ENTREGA_ENTREGOU; i++) {
        r = entregador_passo(&e, m, &es);
        if (robo.pacote_atual != NULL && robo.pacote_atual != &a) {
            violou = true;
        }
    }
    verificar(r == ENTREGA_ENTREGOU, "entregador entrega o primeiro pacote");
    verificar(!violou, "nunca carrega dois nem retira o segundo antes de entregar");
    verificar(a.estado == PACOTE_ENTREGUE, "primeiro pacote foi entregue");
    verificar(es.posicoes[es.tamanho - 1] == &b && b.estado == PACOTE_NA_ESTEIRA,
              "segundo pacote continua no out aguardando");

    mapa_destruir(m);
}

static void teste_conta_entregas(void)
{
    printf("contador de despachos incrementa a cada entrega:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_ENTREGADOR, (Posicao){ 6, 2 }, m);
    Posicao saida = { 4, 2 };
    Posicao despacho = { 1, 2 };
    mapa_definir_tipo(m, despacho, CELULA_PONTO_D);
    Entregador e;
    entregador_inicializar(&e, &robo, saida, despacho);

    Esteira es;
    esteira_inicializar(&es, 4);

    Pacote pacotes[3] = {
        { 0, PACOTE_NA_ESTEIRA, 0 },
        { 1, PACOTE_NA_ESTEIRA, 0 },
        { 2, PACOTE_NA_ESTEIRA, 0 },
    };

    int entregues = 0;
    for (int i = 0; i < 3; i++) {
        colocar_no_out(&es, &pacotes[i]);
        ResultadoEntrega r = ENTREGA_OCIOSO;
        for (int passo = 0; passo < 200 && r != ENTREGA_ENTREGOU; passo++) {
            r = entregador_passo(&e, m, &es);
            if (r == ENTREGA_ENTREGOU) {
                entregues++;
            }
        }
        verificar(r == ENTREGA_ENTREGOU, "entrega o pacote da vez");
        verificar(robo.pacote_atual == NULL && e.estado == ENTREGADOR_OCIOSO,
                  "entregador fica disponível após a entrega");
    }
    verificar(entregues == 3, "uma contagem por entrega, sem perder pacote");

    mapa_destruir(m);
}

int main(void)
{
    teste_inicializacao();
    teste_sem_pacote();
    teste_ciclo_completo();
    teste_um_pacote_por_vez();
    teste_conta_entregas();

    if (falhas > 0) {
        printf("\n%d teste(s) falharam\n", falhas);
        return 1;
    }
    printf("\ntodos os testes passaram\n");
    return 0;
}
