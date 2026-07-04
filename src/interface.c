#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "interface.h"

enum {
    PAR_PADRAO = 1,
    PAR_ESTACAO,
    PAR_DESPACHO,
    PAR_COLETOR,
    PAR_ENTREGADOR,
    PAR_ESTEIRA,
    PAR_TITULO
};

static const char *LEGENDA_TITULO = "Legenda:";
static const char *LEGENDA_CELULAS = "P estacao   D despacho";
static const char *LEGENDA_ROBOS = "C coletor   E entregador";
static const char *LEGENDA_MAPA = "o pacote   . livre   # parede";

char interface_simbolo_celula(TipoCelula tipo)
{
    switch (tipo) {
        case CELULA_PAREDE:    return '#';
        case CELULA_ESTACAO_P: return 'P';
        case CELULA_PONTO_D:   return 'D';
        case CELULA_LIVRE:
        default:               return '.';
    }
}

char interface_simbolo_robo(TipoRobo tipo)
{
    return (tipo == ROBO_COLETOR) ? 'C' : 'E';
}

int interface_compor_mapa(Mapa *mapa, const Robo *robos, int num_robos,
                          char *buffer, int tam)
{
    if (mapa == NULL || buffer == NULL || num_robos < 0 ||
        num_robos > MAX_ROBOS || (num_robos > 0 && robos == NULL)) {
        return -1;
    }

    long necessario = (long)mapa->altura * (mapa->largura + 1) + 1;
    if (tam < necessario) {
        return -1;
    }

    pthread_mutex_lock(&mapa->mutex);
    int pos = 0;
    for (int y = 0; y < mapa->altura; y++) {
        for (int x = 0; x < mapa->largura; x++) {
            char c = interface_simbolo_celula(mapa->celulas[y][x]);
            for (int r = 0; r < num_robos; r++) {
                if (robos[r].posicao.x == x && robos[r].posicao.y == y) {
                    c = interface_simbolo_robo(robos[r].tipo);
                    break;
                }
            }
            buffer[pos++] = c;
        }
        buffer[pos++] = '\n';
    }
    buffer[pos] = '\0';
    pthread_mutex_unlock(&mapa->mutex);
    return pos;
}

int interface_compor_esteira(Esteira *esteira, char *buffer, int tam)
{
    if (esteira == NULL || buffer == NULL) {
        return -1;
    }

    /* "IN[" + posições + "]OUT" + '\0' */
    int necessario = 3 + esteira->tamanho + 4 + 1;
    if (tam < necessario) {
        return -1;
    }

    pthread_mutex_lock(&esteira->mutex);
    int pos = 0;
    buffer[pos++] = 'I';
    buffer[pos++] = 'N';
    buffer[pos++] = '[';
    for (int i = 0; i < esteira->tamanho; i++) {
        buffer[pos++] = (esteira->posicoes[i] != NULL) ? 'o' : '.';
    }
    buffer[pos++] = ']';
    buffer[pos++] = 'O';
    buffer[pos++] = 'U';
    buffer[pos++] = 'T';
    buffer[pos] = '\0';
    pthread_mutex_unlock(&esteira->mutex);
    return pos;
}

bool interface_dimensoes_minimas(const Mapa *mapa, const Esteira *esteira,
                                 int *linhas, int *colunas)
{
    if (mapa == NULL || esteira == NULL || linhas == NULL || colunas == NULL) {
        return false;
    }

    size_t largura_painel = strlen(LEGENDA_CELULAS);
    if (strlen(LEGENDA_ROBOS) > largura_painel) {
        largura_painel = strlen(LEGENDA_ROBOS);
    }
    if (strlen(LEGENDA_MAPA) > largura_painel) {
        largura_painel = strlen(LEGENDA_MAPA);
    }

    int largura_titulo = 46;
    int largura_esteira = 16 + esteira->tamanho;
    int largura_mapa_painel = mapa->largura + 3 + (int)largura_painel;
    *colunas = largura_titulo;
    if (largura_esteira > *colunas) {
        *colunas = largura_esteira;
    }
    if (largura_mapa_painel > *colunas) {
        *colunas = largura_mapa_painel;
    }
    *linhas = mapa->altura + 4;
    if (*linhas < 11) {
        *linhas = 11;
    }
    return true;
}

static void escrever_limitado(int y, int x, const char *texto)
{
    if (texto == NULL || y < 0 || y >= LINES || x < 0 || x >= COLS) {
        return;
    }
    int limite = COLS - x - 1;
    if (limite > 0) {
        mvaddnstr(y, x, texto, limite);
    }
}

void interface_iniciar(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);   /* getch não bloqueia: o laço da simulação segue */
    curs_set(0);

    if (has_colors()) {
        start_color();
        init_pair(PAR_PADRAO,     COLOR_WHITE,   COLOR_BLACK);
        init_pair(PAR_ESTACAO,    COLOR_YELLOW,  COLOR_BLACK);
        init_pair(PAR_DESPACHO,   COLOR_GREEN,   COLOR_BLACK);
        init_pair(PAR_COLETOR,    COLOR_CYAN,    COLOR_BLACK);
        init_pair(PAR_ENTREGADOR, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(PAR_ESTEIRA,    COLOR_BLUE,    COLOR_BLACK);
        init_pair(PAR_TITULO,     COLOR_BLACK,   COLOR_WHITE);
    }
}

void interface_desenhar(Mapa *mapa, const Robo *robos, int num_robos,
                        Esteira *esteira, Estatisticas *stats)
{
    if (mapa == NULL || esteira == NULL || stats == NULL ||
        num_robos < 0 || num_robos > MAX_ROBOS ||
        (num_robos > 0 && robos == NULL)) {
        return;
    }

    int linhas_minimas;
    int colunas_minimas;
    interface_dimensoes_minimas(mapa, esteira,
                                &linhas_minimas, &colunas_minimas);
    if (LINES < linhas_minimas || COLS < colunas_minimas) {
        char aviso[128];
        erase();
        snprintf(aviso, sizeof aviso,
                 "Terminal pequeno: minimo %dx%d, atual %dx%d.",
                 colunas_minimas, linhas_minimas, COLS, LINES);
        escrever_limitado(0, 0, aviso);
        escrever_limitado(2, 0,
                          "Aumente o terminal. Pressione q para sair.");
        refresh();
        return;
    }

    char mapa_txt[4096];
    char belt[3 + MAX_ESTEIRA + 4 + 1];
    if (interface_compor_mapa(mapa, robos, num_robos,
                              mapa_txt, sizeof mapa_txt) < 0 ||
        interface_compor_esteira(esteira, belt, sizeof belt) < 0) {
        return;
    }

    int aguardando;
    int na_esteira;
    int entregues;
    double tempo;
    pthread_mutex_lock(&stats->mutex);
    aguardando = stats->pacotes_aguardando;
    na_esteira = stats->pacotes_na_esteira;
    entregues = stats->pacotes_entregues;
    tempo = stats->tempo_execucao_seg;
    pthread_mutex_unlock(&stats->mutex);

    erase();

    attron(COLOR_PAIR(PAR_TITULO));
    mvprintw(0, 0, " IDP - Centro de Distribuicao ");
    attroff(COLOR_PAIR(PAR_TITULO));
    mvprintw(0, 32, "(q para sair)");

    const int oy = 2;
    for (int y = 0; y < mapa->altura; y++) {
        for (int x = 0; x < mapa->largura; x++) {
            char c = mapa_txt[y * (mapa->largura + 1) + x];
            int par = PAR_PADRAO;
            if (c == 'P') {
                par = PAR_ESTACAO;
            } else if (c == 'D') {
                par = PAR_DESPACHO;
            } else if (c == 'C') {
                par = PAR_COLETOR;
            } else if (c == 'E') {
                par = PAR_ENTREGADOR;
            }
            attron(COLOR_PAIR(par));
            mvaddch(oy + y, x, c);
            attroff(COLOR_PAIR(par));
        }
    }

    int ey = oy + mapa->altura + 1;
    mvprintw(ey, 0, "Esteira: ");
    attron(COLOR_PAIR(PAR_ESTEIRA));
    mvprintw(ey, 9, "%s", belt);
    attroff(COLOR_PAIR(PAR_ESTEIRA));

    int px = mapa->largura + 3;
    mvprintw(2, px, "Aguardando: %d", aguardando);
    mvprintw(3, px, "Na esteira: %d", na_esteira);
    mvprintw(4, px, "Entregues: %d", entregues);
    mvprintw(5, px, "Tempo: %.1f s", tempo);

    mvprintw(7, px,  "%s", LEGENDA_TITULO);
    mvprintw(8, px,  "%s", LEGENDA_CELULAS);
    mvprintw(9, px,  "%s", LEGENDA_ROBOS);
    mvprintw(10, px, "%s", LEGENDA_MAPA);

    refresh();
}

bool interface_tecla_sair(void)
{
    int tecla = getch();
    return tecla == 'q' || tecla == 'Q';
}

void interface_encerrar(void)
{
    curs_set(1);
    endwin();
}
