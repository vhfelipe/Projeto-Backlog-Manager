/* ══════════════════════════════════════════════════════════════════
   historico.h  —  Pilhas LIFO: undo, conquistas, log de status
   ══════════════════════════════════════════════════════════════════ */
#ifndef HISTORICO_H
#define HISTORICO_H

#include "tipos.h"

/* ─── Topos das pilhas ──────────────────────────────────────────── */
extern NoPilhaUndo *pilha_undo;        /* cópia de dados (RF13)     */
extern NoPilhaRef  *pilha_acessos;     /* últimos jogos visualizados */
extern NoPilhaRef  *pilha_zerados;     /* conquistas: zerados (RF14) */
extern NoPilhaRef  *pilha_platinados;  /* conquistas: platinados     */
extern NoPilhaLog  *pilha_log_status;  /* log de transições (RN10)  */

/* ─── Ciclo de vida ─────────────────────────────────────────────── */
void      historico_inicializar(void);
void      historico_liberar_tudo(void);

/* ─── Pilha de Undo (RF13) ──────────────────────────────────────── */
void      historico_push_undo(NoJogo *ref);   /* chamado antes do free */
int       historico_pop_undo(void);           /* restaura último deletado; retorna 1=ok */

/* ─── Últimos Acessos ───────────────────────────────────────────── */
void      historico_push_acesso(NoJogo *ref);

/* ─── Conquistas (RF12 / RF14) ──────────────────────────────────── */
void      historico_push_conquista(NoJogo *ref);  /* decide zerado ou platinado */

/* ─── Log de Status (RN10) ──────────────────────────────────────── */
void      historico_push_log(NoJogo *ref,
                             const char *ant, const char *novo);

/* ─── Manutenção de integridade (RN02) ──────────────────────────── */
void      historico_remover_referencia(NoJogo *ref);

/* ─── Exibição (RF14) ───────────────────────────────────────────── */
void      historico_menu_interativo(void);

#endif /* HISTORICO_H */
