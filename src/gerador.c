#include <stddef.h>

#include "gerador.h"

bool gerador_inicializar(Gerador *gerador, const Cenario *cenario,
                         Mapa *mapa, Estacao *estacoes)
{
    if (cenario == NULL || mapa == NULL || cenario->num_estacoes <= 0 ||
        cenario->num_estacoes > MAX_ESTACOES ||
        cenario->total_pacotes < 0 ||
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
	    for (int j = 0; j < i; j++) {
                estacao_destruir(&estacoes[j]);
            }
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

ResultadoGeracao gerador_gerar(Gerador *gerador, Pacote **out)
{
    if (out != NULL) {
        *out = NULL;
    }
    if (gerador->gerados >= gerador->total_pacotes) {
        return GERACAO_CONCLUIDA;
    }

    /* round-robin com liveness: parte da estação da vez e dá no máximo uma
     * volta completa procurando uma com espaço, em vez de desistir só
     * porque a primeira está cheia. Ao fim de uma volta sem sucesso,
     * proxima_estacao volta ao ponto de partida (rotação líquida zero). */
    for (int i = 0; i < gerador->num_estacoes; i++) {
        Estacao *estacao = &gerador->estacoes[gerador->proxima_estacao];
        if (!estacao_fila_cheia(estacao)) {
            Pacote *pacote = &gerador->pacotes[gerador->gerados];
            pacote->id = gerador->gerados;
            pacote->estado = PACOTE_AGUARDANDO;
            pacote->estacao_origem = gerador->proxima_estacao;

            estacao_enfileirar(estacao, pacote);
            gerador->gerados++;
            gerador->proxima_estacao =
                (gerador->proxima_estacao + 1) % gerador->num_estacoes;
            if (out != NULL) {
                *out = pacote;
            }
            return GERACAO_OK;
        }
        gerador->proxima_estacao =
            (gerador->proxima_estacao + 1) % gerador->num_estacoes;
    }
    return GERACAO_SEM_ESPACO;
}

int gerador_pacotes_restantes(const Gerador *gerador)
{
    return gerador->total_pacotes - gerador->gerados;
}
