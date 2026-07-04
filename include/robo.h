#ifndef ROBO_H
#define ROBO_H

#include "idp.h"
#include "mapa.h"

/* Coloca o robô no mapa na posição inicial e marca a célula como ocupada.
 * A posição precisa estar livre; retorna false (sem posicionar) caso contrário. */
bool robo_inicializar(Robo *robo, int id, TipoRobo tipo, Posicao inicial, Mapa *mapa);

/* Tenta mover o robô uma célula na direção (dx, dy). Só um eixo por vez
 * (sem diagonal): (dx, dy) precisa ser (±1, 0) ou (0, ±1). O movimento só
 * ocorre se a célula de destino estiver dentro dos limites, não for parede e
 * não estiver ocupada por outra entidade. Atualiza a ocupação do mapa.
 * ocupação do destino é atômica (mapa_tentar_ocupar): o destino é tomado antes de liberar a origem. */
bool robo_mover(Robo *robo, Mapa *mapa, int dx, int dy);

/* Dá um passo (uma célula, um eixo) em direção ao alvo: prioriza o eixo mais
 * distante e, se os passos que aproximam estiverem bloqueados, tenta contornar
 * de lado no eixo já alinhado — o que evita o deadlock de corredor entre dois
 * robôs que se cruzam em sentidos opostos. Sem desvio de obstáculos fixos (os
 * cenários atuais não têm paredes internas). Retorna true se o robô se moveu. */
bool robo_passo_em_direcao(Robo *robo, Mapa *mapa, Posicao alvo);

#endif /* ROBO_H */
