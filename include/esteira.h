#ifndef ESTEIRA_H
#define ESTEIRA_H

#include "idp.h"

/* posicoes[0] é a entrada (in), posicoes[tamanho-1] é a saída (out) */

bool esteira_inicializar(Esteira *esteira, int tamanho);
bool esteira_inserir(Esteira *esteira, Pacote *pacote);
bool esteira_avancar(Esteira *esteira);

/* nn muda o estado do pacote, quem decide é o entregador (#8) */
Pacote *esteira_retirar(Esteira *esteira);

int esteira_total(const Esteira *esteira);

#endif 
