// gcc -o programa exemplo-ncurses.c -lncurses
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

void imprime_conteudo(int tipo)
{
    attron(COLOR_PAIR(7));
    move(3, 0);
    printw("Par de cores %02d", tipo);
    move(4, 0);
    attron(COLOR_PAIR(tipo));
    for (int i = '0'; i <= 'z'; i++) {
        printw("%c", i);
    }
    attron(A_REVERSE);
    move(5, 0);
    attron(A_ALTCHARSET);
    for (int i = '0'; i <= 'z'; i++) {
        printw("%c", i);
    }
    attroff(A_ALTCHARSET);
    attroff(A_REVERSE);
    refresh();
}

int main()
{
    int tecla;
    initscr(); // Inicializa a tela (posição atual é (0, 0))
    start_color();
    raw();    // Não precisa esperar uma quebra de linha
    noecho(); // O que for digitado não aparece na tela
    keypad(stdscr, TRUE); // Teclas especiais como F1, F2, etc..

    // Cria as cores que serão utilizadas (mas ainda não usa)
    init_pair(7, COLOR_BLACK, COLOR_WHITE); // padrão
    init_pair(1, COLOR_BLUE, COLOR_WHITE);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_RED, COLOR_BLACK);

    printw("Iniciando...");
    refresh(); // Realiza todos os comandos pendentes atualizando na tela
    sleep(1);

    attron(COLOR_PAIR(5));
    mvprintw(LINES - 1, 0, "(%d - %d) - Pressione 'q' para sair...", LINES, COLS);
    attroff(COLOR_PAIR(5));

    move(0, 0);
    printw(" - Digite 1, 2, q ou F4: ");
    do {
        tecla = getch();
        switch (tecla) {
            case '1':
                imprime_conteudo(1);
                break;
            case '2':
                imprime_conteudo(2);
                break;
            case KEY_F(4):
                attron(COLOR_PAIR(7));
                move(8, 0);
                printw("Tecla F4 foi pressionada.. saindo");
                refresh();
                sleep(1);
            case 'q':
            default:
                break;
        }
    } while (tecla != 'q' && tecla != KEY_F(4));

    // Desabilitando tudo que foi habilitado antes de sair, para não deixar o
    // terminal em estado diferente do anterior à execução
    keypad(stdscr, FALSE);
    noraw();
    echo();

    endwin();

    printf("Termino da execucao\n");

    return 0;
}
