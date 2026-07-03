#include <ncurses.h>

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

int interface_compor_mapa(const Mapa *mapa, const Robo *robos, int num_robos,
                          char *buffer, int tam)
{
    if (mapa == NULL || buffer == NULL || (num_robos > 0 && robos == NULL)) {
        return -1;
    }

    long necessario = (long)mapa->altura * (mapa->largura + 1) + 1;
    if (tam < necessario) {
        return -1;
    }

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
    return pos;
}

int interface_compor_esteira(const Esteira *esteira, char *buffer, int tam)
{
    if (esteira == NULL || buffer == NULL) {
        return -1;
    }

    /* "IN[" + posições + "]OUT" + '\0' */
    int necessario = 3 + esteira->tamanho + 4 + 1;
    if (tam < necessario) {
        return -1;
    }

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
    return pos;
}

static int par_da_celula(TipoCelula tipo)
{
    switch (tipo) {
        case CELULA_ESTACAO_P: return PAR_ESTACAO;
        case CELULA_PONTO_D:   return PAR_DESPACHO;
        default:               return PAR_PADRAO;
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

void interface_desenhar(const Mapa *mapa, const Robo *robos, int num_robos,
                        const Esteira *esteira, const Estatisticas *stats)
{
    erase();

    attron(COLOR_PAIR(PAR_TITULO));
    mvprintw(0, 0, " IDP - Centro de Distribuicao ");
    attroff(COLOR_PAIR(PAR_TITULO));
    mvprintw(0, 32, "(q para sair)");

    const int oy = 2;
    for (int y = 0; y < mapa->altura; y++) {
        for (int x = 0; x < mapa->largura; x++) {
            char c = interface_simbolo_celula(mapa->celulas[y][x]);
            int par = par_da_celula(mapa->celulas[y][x]);
            for (int r = 0; r < num_robos; r++) {
                if (robos[r].posicao.x == x && robos[r].posicao.y == y) {
                    c = interface_simbolo_robo(robos[r].tipo);
                    par = (robos[r].tipo == ROBO_COLETOR) ? PAR_COLETOR
                                                          : PAR_ENTREGADOR;
                    break;
                }
            }
            attron(COLOR_PAIR(par));
            mvaddch(oy + y, x, c);
            attroff(COLOR_PAIR(par));
        }
    }

    int ey = oy + mapa->altura + 1;
    char belt[3 + MAX_ESTEIRA + 4 + 1];
    mvprintw(ey, 0, "Esteira: ");
    if (interface_compor_esteira(esteira, belt, sizeof belt) > 0) {
        attron(COLOR_PAIR(PAR_ESTEIRA));
        mvprintw(ey, 9, "%s", belt);
        attroff(COLOR_PAIR(PAR_ESTEIRA));
    }

    int px = mapa->largura + 3;
    mvprintw(2, px, "Pacotes aguardando coleta : %d", stats->pacotes_aguardando);
    mvprintw(3, px, "Pacotes na esteira        : %d", stats->pacotes_na_esteira);
    mvprintw(4, px, "Pacotes entregues         : %d", stats->pacotes_entregues);
    mvprintw(5, px, "Tempo de execucao (s)     : %.1f", stats->tempo_execucao_seg);

    mvprintw(7, px,  "Legenda:");
    mvprintw(8, px,  "P estacao de coleta   D ponto de despacho");
    mvprintw(9, px,  "C robo coletor        E robo entregador");
    mvprintw(10, px, "o pacote na esteira   . livre   # parede");

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
