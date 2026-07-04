#ifndef INTERFACE_H
#define INTERFACE_H

#include "idp.h"

/* Interface de acompanhamento em tempo real (ncurses, Apêndice A do enunciado).
 *
 * Cada parte mutável do frame é copiada sob o mutex do recurso correspondente
 * antes de ser desenhada. Somente a thread principal chama ncurses.
 *
 * As funções de composição (interface_compor_*) não dependem de ncurses: são
 * usadas tanto pelo backend gráfico quanto pelos testes e pela saída em texto
 * quando não há terminal interativo. */

/* Símbolo usado para cada tipo de célula do mapa e para cada tipo de robô. */
char interface_simbolo_celula(TipoCelula tipo);
char interface_simbolo_robo(TipoRobo tipo);

/* Compõe o mapa (células com os robôs sobrepostos) como texto: uma linha do
 * mapa por linha, separadas por '\n' e terminada em '\0'. Retorna o número de
 * bytes escritos (sem o '\0'), ou -1 se algum ponteiro for nulo ou o buffer
 * não comportar o mapa inteiro. */
int interface_compor_mapa(Mapa *mapa, const Robo *robos, int num_robos,
                          char *buffer, int tam);

/* Compõe a esteira como "IN[..o.o..]OUT": '.' posição livre, 'o' ocupada,
 * da entrada (in) à saída (out). Retorna bytes escritos (sem o '\0'), ou -1
 * se algo for nulo ou o buffer for pequeno demais. */
int interface_compor_esteira(Esteira *esteira, char *buffer, int tam);

/* Calcula o menor terminal capaz de exibir o frame sem quebra de linha.
 * Retorna false para argumentos nulos. */
bool interface_dimensoes_minimas(const Mapa *mapa, const Esteira *esteira,
                                 int *linhas, int *colunas);

/* Inicializa o modo ncurses: tela, cores, cursor oculto e getch não-bloqueante. */
void interface_iniciar(void);

/* Desenha um frame completo: mapa + robôs + esteira + painel de estatísticas
 * (aguardando / na esteira / entregues / tempo). */
void interface_desenhar(Mapa *mapa, const Robo *robos, int num_robos,
                        Esteira *esteira, Estatisticas *stats);

/* true se o usuário pediu para sair (tecla 'q'). Não bloqueia. */
bool interface_tecla_sair(void);

/* Restaura o terminal ao estado anterior à interface. */
void interface_encerrar(void);

#endif /* INTERFACE_H */
