/* ══════════════════════════════════════════════════════════════════
   plataformas.h  —  Lista canônica de plataformas válidas

   Resolve dois problemas identificados nos testes:
     1. Normalização: "PlayStation", "playstation", "PLAYSTATION"
        → sempre "PlayStation" (um único carrossel por plataforma)
     2. Validação: entradas arbitrárias como "Pizza" retornam NULL
   ══════════════════════════════════════════════════════════════════ */
#ifndef PLATAFORMAS_H
#define PLATAFORMAS_H

#include "tipos.h"

/* Retorna o nome canônico da plataforma, ou NULL se não reconhecida.
   Comparação é case-insensitive e ignora espaços nas bordas.        */
const char *plataforma_normalizar(const char *entrada);

/* Exibe no terminal a lista completa de plataformas aceitas.        */
void plataforma_listar_validas(void);

#endif /* PLATAFORMAS_H */
