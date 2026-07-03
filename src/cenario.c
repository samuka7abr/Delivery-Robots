#include <stddef.h>

#include "cenario.h"

/*
 * | Parâmetro            | 0 - Básico | 1 - Intermediário | 2 - Estresse |
 * |----------------------|------------|-------------------|--------------|
 * | Mapa (larg x alt)    | 20 x 10    | 30 x 15           | 40 x 20      |
 * | Robôs coletores      | 2          | 4                 | 6            |
 * | Robôs entregadores   | 2          | 3                 | 5            |
 * | Estações P           | 2          | 3                 | 4            |
 * | Pontos D             | 2          | 3                 | 4            |
 * | Tamanho da esteira   | 8          | 12                | 20           |
 * | Total de pacotes     | 20         | 50                | 100          |
 *
 * 0 - Básico: poucos robôs e pouca disputa; valida o fluxo
 *     coleta -> esteira -> entrega no caso mais simples.
 * 1 - Intermediário: mais robôs que estações/pontos, gerando disputa
 *     moderada pelos recursos compartilhados.
 * 2 - Estresse: mapa grande e muitos robôs disputando esteira, estações
 *     e pontos D; cenário para expor race conditions e gargalos.
 */
static const Cenario cenarios[CENARIO_TOTAL] = {
    {
        .largura_mapa           = 20,
        .altura_mapa            = 10,
        .num_robos_coletores    = 2,
        .num_robos_entregadores = 2,
        .num_estacoes           = 2,
        .num_pontos_despacho    = 2,
        .tamanho_esteira        = 8,
        .total_pacotes          = 20,
    },
    {
        .largura_mapa           = 30,
        .altura_mapa            = 15,
        .num_robos_coletores    = 4,
        .num_robos_entregadores = 3,
        .num_estacoes           = 3,
        .num_pontos_despacho    = 3,
        .tamanho_esteira        = 12,
        .total_pacotes          = 50,
    },
    {
        .largura_mapa           = 40,
        .altura_mapa            = 20,
        .num_robos_coletores    = 6,
        .num_robos_entregadores = 5,
        .num_estacoes           = 4,
        .num_pontos_despacho    = 4,
        .tamanho_esteira        = 20,
        .total_pacotes          = 100,
    },
};

const Cenario *cenario_obter(int indice)
{
    if (indice < 0 || indice >= CENARIO_TOTAL) {
        return NULL;
    }
    return &cenarios[indice];
}
