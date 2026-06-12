/* ══════════════════════════════════════════════════════════════════
   carrossel.h  —  Listas circulares duplamente encadeadas com sentinela
   ══════════════════════════════════════════════════════════════════ */
#ifndef CARROSSEL_H
#define CARROSSEL_H

#include "tipos.h"

/* ─── Estado global ─────────────────────────────────────────────── */
extern Carrossel *carrossel_lista;   /* cabeça da lista de carrosseis */

/* ─── Ciclo de vida ─────────────────────────────────────────────── */
void       carrossel_inicializar(void);
void       carrossel_liberar_tudo(void);

/* ─── Operações sobre carrosseis ────────────────────────────────── */
Carrossel *carrossel_obter(const char *plataforma);   /* cria se não existir */
void       carrossel_inserir(NoJogo *ref);            /* global + plataforma */
void       carrossel_remover_referencia(NoJogo *ref); /* chamado por catalogo */
void       carrossel_migrar(NoJogo *ref);             /* troca de plataforma (RN07) */

/* ─── Navegação (RF06 / RF07) ───────────────────────────────────── */
void       carrossel_navegar(const char *plataforma); /* loop interativo */
void       carrossel_listar_plataformas(void);

#endif /* CARROSSEL_H */
