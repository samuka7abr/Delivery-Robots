#ifndef ESTEIRA_H
#define ESTEIRA_H

#include "idp.h"

/* posicoes[0] é a entrada (in), posicoes[tamanho-1] é a saída (out).
 * Todas as operações são protegidas pelo mutex da esteira; a cond var
 * sinaliza mudanças no in (liberou) e no out (chegou pacote). */

bool esteira_inicializar(Esteira *esteira, int tamanho);

/* Destrói mutex e cond var. Chamar uma vez, no encerramento. */
void esteira_destruir(Esteira *esteira);

bool esteira_inserir(Esteira *esteira, Pacote *pacote);
bool esteira_avancar(Esteira *esteira);

/* nn muda o estado do pacote, quem decide é o entregador (#8) */
Pacote *esteira_retirar(Esteira *esteira);

int esteira_total(Esteira *esteira);

#endif
