#include <stdio.h>
#include <string.h>

#include "interface.h"
#include "mapa.h"
#include "robo.h"
#include "esteira.h"

static int falhas = 0;

static void verificar(bool condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "ok" : "FALHOU", descricao);
    if (!condicao) {
        falhas++;
    }
}

static void teste_simbolos(void)
{
    printf("símbolos de célula e robô:\n");
    verificar(interface_simbolo_celula(CELULA_LIVRE) == '.', "livre -> '.'");
    verificar(interface_simbolo_celula(CELULA_PAREDE) == '#', "parede -> '#'");
    verificar(interface_simbolo_celula(CELULA_ESTACAO_P) == 'P', "estação -> 'P'");
    verificar(interface_simbolo_celula(CELULA_PONTO_D) == 'D', "ponto D -> 'D'");
    verificar(interface_simbolo_robo(ROBO_COLETOR) == 'C', "coletor -> 'C'");
    verificar(interface_simbolo_robo(ROBO_ENTREGADOR) == 'E', "entregador -> 'E'");
}

static void teste_compor_mapa(void)
{
    printf("composição do mapa com robôs sobrepostos:\n");
    Mapa *mapa = mapa_criar(3, 2);
    verificar(mapa != NULL, "cria mapa 3x2");

    mapa_definir_tipo(mapa, (Posicao){ 1, 0 }, CELULA_ESTACAO_P);
    mapa_definir_tipo(mapa, (Posicao){ 2, 1 }, CELULA_PONTO_D);

    Robo robos[2] = {
        { .id = 0, .tipo = ROBO_COLETOR,    .posicao = { 0, 0 }, .pacote_atual = NULL },
        { .id = 1, .tipo = ROBO_ENTREGADOR, .posicao = { 1, 1 }, .pacote_atual = NULL },
    };

    char buffer[32];
    int n = interface_compor_mapa(mapa, robos, 2, buffer, sizeof buffer);
    verificar(n == 8, "retorna 8 bytes (2 linhas de 3 + 2 quebras)");
    verificar(strcmp(buffer, "CP.\n.ED\n") == 0, "robô sobrepõe a célula do mapa");

    verificar(interface_compor_mapa(NULL, robos, 2, buffer, sizeof buffer) == -1,
              "recusa mapa nulo");
    verificar(interface_compor_mapa(mapa, NULL, 2, buffer, sizeof buffer) == -1,
              "recusa robôs nulos com num_robos > 0");
    verificar(interface_compor_mapa(mapa, robos, -1, buffer, sizeof buffer) == -1,
              "recusa quantidade negativa de robôs");
    verificar(interface_compor_mapa(mapa, robos, MAX_ROBOS + 1,
                                    buffer, sizeof buffer) == -1,
              "recusa quantidade de robôs acima do limite");
    verificar(interface_compor_mapa(mapa, robos, 2, buffer, 4) == -1,
              "recusa buffer pequeno demais");

    /* sem robôs: só as células do mapa */
    int m = interface_compor_mapa(mapa, NULL, 0, buffer, sizeof buffer);
    verificar(m == 8 && strcmp(buffer, ".P.\n..D\n") == 0,
              "compõe sem robôs (num_robos = 0, robos NULL)");

    mapa_destruir(mapa);
}

static void teste_compor_esteira(void)
{
    printf("composição da esteira:\n");
    Esteira e;
    esteira_inicializar(&e, 8);

    char buffer[32];
    int n = interface_compor_esteira(&e, buffer, sizeof buffer);
    verificar(n == 15 && strcmp(buffer, "IN[........]OUT") == 0,
              "esteira vazia: 8 posições livres");

    Pacote p = { 0, PACOTE_AGUARDANDO, 0 };
    esteira_inserir(&e, &p);
    interface_compor_esteira(&e, buffer, sizeof buffer);
    verificar(strcmp(buffer, "IN[o.......]OUT") == 0,
              "pacote na entrada aparece como 'o' na posição 0");

    esteira_avancar(&e);
    interface_compor_esteira(&e, buffer, sizeof buffer);
    verificar(strcmp(buffer, "IN[.o......]OUT") == 0,
              "após avançar, o pacote anda uma posição");

    verificar(interface_compor_esteira(NULL, buffer, sizeof buffer) == -1,
              "recusa esteira nula");
    verificar(interface_compor_esteira(&e, buffer, 4) == -1,
              "recusa buffer pequeno demais");

    esteira_destruir(&e);
}

typedef struct {
    Robo *robo;
    Mapa *mapa;
} ArgsMovimento;

static void *mover_robo_repetidamente(void *arg)
{
    ArgsMovimento *args = arg;
    for (int i = 0; i < 5000; i++) {
        robo_mover(args->robo, args->mapa, 1, 0);
        robo_mover(args->robo, args->mapa, -1, 0);
    }
    return NULL;
}

static void teste_snapshot_concorrente(void)
{
    printf("snapshot do mapa durante movimentação concorrente:\n");
    Mapa *mapa = mapa_criar(3, 1);
    Robo robo;
    verificar(mapa != NULL, "cria mapa para o teste concorrente");
    if (mapa == NULL) {
        return;
    }
    bool inicializou = robo_inicializar(&robo, 0, ROBO_COLETOR,
                                        (Posicao){ 1, 0 }, mapa);
    verificar(inicializou, "inicializa robô para o teste concorrente");
    if (!inicializou) {
        mapa_destruir(mapa);
        return;
    }

    ArgsMovimento args = { &robo, mapa };
    pthread_t thread;
    pthread_create(&thread, NULL, mover_robo_repetidamente, &args);

    bool snapshots_validos = true;
    for (int i = 0; i < 5000; i++) {
        char buffer[8];
        interface_compor_mapa(mapa, &robo, 1, buffer, sizeof buffer);
        int robos_visiveis = 0;
        for (int j = 0; buffer[j] != '\0'; j++) {
            robos_visiveis += buffer[j] == 'C';
        }
        if (robos_visiveis != 1) {
            snapshots_validos = false;
            break;
        }
    }

    pthread_join(thread, NULL);
    verificar(snapshots_validos,
              "cada snapshot contém exatamente uma posição do robô");
    mapa_destruir(mapa);
}

static void teste_layout_cenario_maior(void)
{
    printf("layout do maior cenário em terminal padrão:\n");
    Mapa *mapa = mapa_criar(40, 20);
    Esteira esteira;
    verificar(mapa != NULL, "cria mapa do cenário 2");
    if (mapa == NULL) {
        return;
    }
    bool inicializou_esteira = esteira_inicializar(&esteira, 20);
    verificar(inicializou_esteira, "inicializa esteira do cenário 2");
    if (!inicializou_esteira) {
        mapa_destruir(mapa);
        return;
    }

    int linhas;
    int colunas;
    verificar(interface_dimensoes_minimas(mapa, &esteira, &linhas, &colunas),
              "calcula dimensões mínimas");
    verificar(linhas <= 24 && colunas <= 80,
              "cenário 2 cabe sem sobreposição em terminal 80x24");
    verificar(!interface_dimensoes_minimas(NULL, &esteira, &linhas, &colunas),
              "recusa mapa nulo ao calcular layout");

    esteira_destruir(&esteira);
    mapa_destruir(mapa);
}

int main(void)
{
    teste_simbolos();
    teste_compor_mapa();
    teste_compor_esteira();
    teste_snapshot_concorrente();
    teste_layout_cenario_maior();

    printf("\n%s\n", falhas == 0 ? "todos os testes passaram" : "HÁ FALHAS");
    return falhas != 0;
}
