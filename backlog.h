/* ══════════════════════════════════════════════════════════════════
   backlog.h  —  Filas FIFO: "A Jogar" e "A Zerar"
   ══════════════════════════════════════════════════════════════════ */
#ifndef BACKLOG_H
#define BACKLOG_H

#include "tipos.h"

/* ─── Estado global — duas filas independentes ──────────────────── */
extern Fila backlog_filas[2];  /* [FILA_A_JOGAR] e [FILA_A_ZERAR] */

/* ─── Ciclo de vida ─────────────────────────────────────────────── */
void    backlog_inicializar(void);
void    backlog_liberar_tudo(void);

/* ─── Operações FIFO (RN04) ─────────────────────────────────────── */
int     backlog_enqueue(int fila_idx, NoJogo *ref);   /* entra no tail */
NoJogo *backlog_dequeue(int fila_idx);                /* sai do head   */
NoJogo *backlog_peek(int fila_idx);                   /* consulta head sem remover */

/* ─── Manutenção de integridade (RN02) ──────────────────────────── */
void    backlog_remover_referencia(NoJogo *ref);      /* chamado por catalogo */

/* ─── Exibição (RF11) ───────────────────────────────────────────── */
void    backlog_exibir_fila(int fila_idx);
void    backlog_menu_interativo(void);

#endif /* BACKLOG_H */
