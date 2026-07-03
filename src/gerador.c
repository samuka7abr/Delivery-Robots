#include <stddef.h>

#include "gerador.h"

bool gerador_inicializar(Gerador *gerador, const Cenario *cenario,
                         Mapa *mapa, Estacao *estacoes)
{
    if (cenario == NULL || cenario->num_estacoes <= 0 ||
        cenario->num_estacoes > MAX_ESTACOES ||
        cenario->total_pacotes > MAX_PACOTES) {
        return false;
    }

    for (int i = 0; i < cenario->num_estacoes; i++) {
        Posicao p = {
            1,
            (i + 1) * cenario->altura_mapa / (cenario->num_estacoes + 1),
        };
        if (!mapa_dentro_limites(mapa, p) ||
            mapa->celulas[p.y][p.x] != CELULA_LIVRE) {
            return false;
        }
        estacao_inicializar(&estacoes[i], p);
        mapa_definir_tipo(mapa, p, CELULA_ESTACAO_P);
    }

    gerador->estacoes = estacoes;
    gerador->num_estacoes = cenario->num_estacoes;
    gerador->total_pacotes = cenario->total_pacotes;
    gerador->gerados = 0;
    gerador->proxima_estacao = 0;
    return true;
}

Pacote *gerador_gerar(Gerador *gerador)
{
    if (gerador->gerados >= gerador->total_pacotes) {
        return NULL;
    }

    Estacao *estacao = &gerador->estacoes[gerador->proxima_estacao];
    if (estacao_fila_cheia(estacao)) {
        return NULL;
    }

    Pacote *pacote = &gerador->pacotes[gerador->gerados];
    pacote->id = gerador->gerados;
    pacote->estado = PACOTE_AGUARDANDO;
    pacote->estacao_origem = gerador->proxima_estacao;

    estacao_enfileirar(estacao, pacote);
    gerador->gerados++;
    gerador->proxima_estacao =
        (gerador->proxima_estacao + 1) % gerador->num_estacoes;
    return pacote;
}

int gerador_pacotes_restantes(const Gerador *gerador)
{
    return gerador->total_pacotes - gerador->gerados;
}
