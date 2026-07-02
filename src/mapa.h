#ifndef MAPA_H
#define MAPA_H

#include "idp.h"

/* Cria o mapa (largura x altura) com todas as células livres e sem ocupação.
 * Aloca as matrizes de tipo e de ocupação. O mutex do mapa não é
 * inicializado aqui — fica pra Issue #9, quando threads entram em cena.
 * Retorna NULL em caso de falha de alocação. */
Mapa *mapa_criar(int largura, int altura);

/* Libera as matrizes do mapa e a própria struct. Não mexe no mutex, que
 * ainda não foi inicializado nesta etapa sequencial (ver Issue #9). */
void mapa_destruir(Mapa *mapa);

/* Define o tipo fixo de uma célula (parede, estação P, ponto D, livre). */
void mapa_definir_tipo(Mapa *mapa, Posicao p, TipoCelula tipo);

/* Verdadeiro se a posição está dentro dos limites do mapa. */
bool mapa_dentro_limites(const Mapa *mapa, Posicao p);

/* Verdadeiro se a célula pode receber uma entidade agora: dentro dos limites,
 * não é parede e não está ocupada. Checagem sequencial, sem lock. */
bool mapa_celula_livre(const Mapa *mapa, Posicao p);

/* Marca/desmarca a ocupação de uma célula por uma entidade. */
void mapa_ocupar(Mapa *mapa, Posicao p);
void mapa_liberar(Mapa *mapa, Posicao p);

#endif /* MAPA_H */
