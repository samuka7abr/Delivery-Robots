#include <stdio.h>

#include "robo.h"

static int falhas = 0;

static void verificar(bool condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "ok" : "FALHOU", descricao);
    if (!condicao) {
        falhas++;
    }
}

static void teste_passo_reto(void)
{
    printf("passo em direção com caminho livre:\n");
    Mapa *m = mapa_criar(6, 5);
    Robo r;
    robo_inicializar(&r, 0, ROBO_COLETOR, (Posicao){ 1, 2 }, m);

    verificar(robo_passo_em_direcao(&r, m, (Posicao){ 4, 2 }),
              "anda quando o caminho está livre");
    verificar(r.posicao.x == 2 && r.posicao.y == 2,
              "reduz o eixo mais distante (anda para a direita)");

    mapa_destruir(m);
}

/* Regressão do deadlock de corredor: dois robôs alinhados na mesma linha,
 * cada um querendo passar pela célula do outro. Sem o desvio de lado eles se
 * travariam mutuamente; com ele, um sai da linha e o corredor destrava. */
static void teste_desvio_corredor(void)
{
    printf("desvio em corredor de sentidos opostos:\n");
    Mapa *m = mapa_criar(7, 5);
    Robo a, b;
    robo_inicializar(&a, 0, ROBO_ENTREGADOR, (Posicao){ 2, 2 }, m);
    robo_inicializar(&b, 1, ROBO_ENTREGADOR, (Posicao){ 3, 2 }, m);

    /* a quer ir para a direita (alvo à direita de b); b quer ir para a esquerda
     * (alvo à esquerda de a): cara a cara na linha y=2 */
    bool moveu_a = robo_passo_em_direcao(&a, m, (Posicao){ 5, 2 });
    verificar(moveu_a, "robô bloqueado de frente ainda consegue se mover");
    verificar(a.posicao.y != 2, "sai da linha do corredor (desvio lateral)");

    bool moveu_b = robo_passo_em_direcao(&b, m, (Posicao){ 0, 2 });
    verificar(moveu_b, "o segundo robô avança pela célula liberada");
    verificar(b.posicao.x == 2 && b.posicao.y == 2,
              "ocupa a célula que o primeiro desocupou");

    mapa_destruir(m);
}

static void teste_sem_saida(void)
{
    printf("robô sem nenhuma célula livre ao redor:\n");
    Mapa *m = mapa_criar(5, 5);
    Robo r;
    robo_inicializar(&r, 0, ROBO_COLETOR, (Posicao){ 2, 2 }, m);
    /* cerca as quatro células vizinhas */
    mapa_ocupar(m, (Posicao){ 1, 2 });
    mapa_ocupar(m, (Posicao){ 3, 2 });
    mapa_ocupar(m, (Posicao){ 2, 1 });
    mapa_ocupar(m, (Posicao){ 2, 3 });

    verificar(!robo_passo_em_direcao(&r, m, (Posicao){ 0, 2 }),
              "não se move quando está totalmente cercado");
    verificar(r.posicao.x == 2 && r.posicao.y == 2, "permanece na posição");

    mapa_destruir(m);
}

int main(void)
{
    teste_passo_reto();
    teste_desvio_corredor();
    teste_sem_saida();

    if (falhas > 0) {
        printf("\n%d teste(s) falharam\n", falhas);
        return 1;
    }
    printf("\ntodos os testes passaram\n");
    return 0;
}
