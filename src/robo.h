#ifndef ROBO_H
#define ROBO_H

#include "idp.h"
#include "mapa.h"

/* Coloca o robô no mapa na posição inicial e marca a célula como ocupada.
 * A posição precisa estar livre; retorna false (sem posicionar) caso contrário. */
bool robo_inicializar(Robo *robo, int id, TipoRobo tipo, Posicao inicial, Mapa *mapa);

/* Tenta mover o robô uma célula na direção (dx, dy). O movimento só ocorre se a
 * célula de destino estiver dentro dos limites, não for parede e não estiver
 * ocupada por outra entidade. Atualiza a ocupação do mapa. Checagem sequencial,
 * sem lock. Retorna true se o robô se moveu. */
bool robo_mover(Robo *robo, Mapa *mapa, int dx, int dy);

#endif /* ROBO_H */
