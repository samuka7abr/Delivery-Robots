#ifndef CENARIO_H
#define CENARIO_H

#include "idp.h"

#define CENARIO_TOTAL 3

/* Retorna o cenário estático de índice [0, CENARIO_TOTAL)*/
const Cenario *cenario_obter(int indice);

#endif 
