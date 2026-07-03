#include <stdio.h>

#include "coletor.h"

static int falhas = 0;

static void verificar(bool condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "ok" : "FALHOU", descricao);
    if (!condicao) {
        falhas++;
    }
}

/* Roda passos até o desfecho alvo (ou o limite). Retorna o último resultado. */
static ResultadoColeta rodar_ate(Coletor *c, Mapa *m, Estacao *e, int n,
                                 Esteira *es, ResultadoColeta alvo, int max)
{
    ResultadoColeta r = COLETA_OCIOSO;
    for (int i = 0; i < max; i++) {
        r = coletor_passo(c, m, e, n, es);
        if (r == alvo) {
            return r;
        }
    }
    return r;
}

static void teste_inicializacao(void)
{
    printf("inicialização do coletor:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    Posicao inicial = { 6, 2 };
    robo_inicializar(&robo, 0, ROBO_COLETOR, inicial, m);

    Posicao entrada = { 4, 2 };
    Coletor c;
    coletor_inicializar(&c, &robo, entrada);
    verificar(c.estado == COLETOR_OCIOSO, "coletor começa ocioso");
    verificar(c.robo == &robo && c.robo->pacote_atual == NULL,
              "coletor começa sem pacote");

    mapa_destruir(m);
}

static void teste_sem_pacote(void)
{
    printf("nenhuma estação com pacote:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_COLETOR, (Posicao){ 6, 2 }, m);
    Coletor c;
    coletor_inicializar(&c, &robo, (Posicao){ 4, 2 });

    Estacao estacao;
    estacao_inicializar(&estacao, (Posicao){ 1, 2 });

    Esteira e;
    esteira_inicializar(&e, 4);

    verificar(coletor_passo(&c, m, &estacao, 1, &e) == COLETA_OCIOSO,
              "sem pacote na estação o coletor fica ocioso");
    verificar(c.estado == COLETOR_OCIOSO, "estado permanece ocioso");

    mapa_destruir(m);
}

static void teste_ciclo_completo(void)
{
    printf("ciclo completo estação -> in da esteira:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_COLETOR, (Posicao){ 6, 4 }, m);
    Posicao entrada = { 5, 2 };
    Coletor c;
    coletor_inicializar(&c, &robo, entrada);

    Estacao estacao;
    estacao_inicializar(&estacao, (Posicao){ 1, 2 });
    Pacote p = { 7, PACOTE_AGUARDANDO, 0 };
    estacao_enfileirar(&estacao, &p);

    Esteira e;
    esteira_inicializar(&e, 4);

    verificar(rodar_ate(&c, m, &estacao, 1, &e, COLETA_COLETOU, 100) == COLETA_COLETOU,
              "coletor alcança a estação e coleta o pacote");
    verificar(robo.pacote_atual == &p && p.estado == PACOTE_TRANSPORTADO,
              "pacote fica em transporte com o robô");
    verificar(estacao_fila_vazia(&estacao), "pacote sai da fila da estação");

    verificar(rodar_ate(&c, m, &estacao, 1, &e, COLETA_INSERIU, 100) == COLETA_INSERIU,
              "coletor leva o pacote até o in e insere");
    verificar(e.posicoes[0] == &p && p.estado == PACOTE_NA_ESTEIRA,
              "pacote entra na esteira pelo in");
    verificar(robo.pacote_atual == NULL && c.estado == COLETOR_OCIOSO,
              "coletor entrega e volta a ficar ocioso");

    mapa_destruir(m);
}

static void teste_um_pacote_por_vez(void)
{
    printf("no máximo um pacote por vez:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_COLETOR, (Posicao){ 6, 4 }, m);
    Coletor c;
    coletor_inicializar(&c, &robo, (Posicao){ 5, 2 });

    Estacao estacao;
    estacao_inicializar(&estacao, (Posicao){ 1, 2 });
    Pacote a = { 0, PACOTE_AGUARDANDO, 0 };
    Pacote b = { 1, PACOTE_AGUARDANDO, 0 };
    estacao_enfileirar(&estacao, &a);
    estacao_enfileirar(&estacao, &b);

    Esteira e;
    esteira_inicializar(&e, 4);

    rodar_ate(&c, m, &estacao, 1, &e, COLETA_COLETOU, 100);
    verificar(robo.pacote_atual == &a, "coleta o primeiro pacote (FIFO)");
    verificar(estacao.total == 1, "só um pacote sai da fila por coleta");

    /* até entregar o primeiro, o robô nunca carrega o segundo nem o retira da
     * fila — invariante checado a cada passo, sem depender de contagem fixa */
    bool violou = false;
    ResultadoColeta r = COLETA_OCIOSO;
    for (int i = 0; i < 100 && r != COLETA_INSERIU; i++) {
        r = coletor_passo(&c, m, &estacao, 1, &e);
        if (robo.pacote_atual != NULL && robo.pacote_atual != &a) {
            violou = true;
        }
        if (estacao.total < 1) {
            violou = true;
        }
    }
    verificar(r == COLETA_INSERIU, "coletor entrega o primeiro pacote");
    verificar(!violou, "nunca carrega dois nem coleta o segundo antes de entregar");
    verificar(e.posicoes[0] == &a, "só o primeiro pacote entrou na esteira");
    verificar(estacao.total == 1, "segundo pacote continua aguardando após a entrega");

    mapa_destruir(m);
}

static void teste_aguarda_in_ocupado(void)
{
    printf("aguarda quando o in está ocupado:\n");
    Mapa *m = mapa_criar(8, 5);
    Robo robo;
    robo_inicializar(&robo, 0, ROBO_COLETOR, (Posicao){ 6, 4 }, m);
    Posicao entrada = { 5, 2 };
    Coletor c;
    coletor_inicializar(&c, &robo, entrada);

    Estacao estacao;
    estacao_inicializar(&estacao, (Posicao){ 1, 2 });
    Pacote p = { 9, PACOTE_AGUARDANDO, 0 };
    estacao_enfileirar(&estacao, &p);

    Esteira e;
    esteira_inicializar(&e, 4);
    /* ocupa o in previamente com outro pacote */
    Pacote bloqueio = { 99, PACOTE_AGUARDANDO, 0 };
    esteira_inserir(&e, &bloqueio);

    verificar(rodar_ate(&c, m, &estacao, 1, &e, COLETA_AGUARDANDO_IN, 200)
                  == COLETA_AGUARDANDO_IN,
              "coletor chega no in ocupado e aguarda");
    verificar(robo.pacote_atual == &p && c.estado == COLETOR_INDO_ESTEIRA,
              "coletor segura o pacote enquanto aguarda");
    verificar(e.posicoes[0] == &bloqueio, "pacote em transporte não sobrescreve o in");

    /* libera o in avançando a esteira e o coletor consegue inserir */
    esteira_avancar(&e);
    verificar(rodar_ate(&c, m, &estacao, 1, &e, COLETA_INSERIU, 5) == COLETA_INSERIU,
              "liberado o in, o coletor insere o pacote que segurava");
    verificar(e.posicoes[0] == &p && robo.pacote_atual == NULL,
              "pacote antes retido entra na esteira");

    mapa_destruir(m);
}

int main(void)
{
    teste_inicializacao();
    teste_sem_pacote();
    teste_ciclo_completo();
    teste_um_pacote_por_vez();
    teste_aguarda_in_ocupado();

    if (falhas > 0) {
        printf("\n%d teste(s) falharam\n", falhas);
        return 1;
    }
    printf("\ntodos os testes passaram\n");
    return 0;
}
