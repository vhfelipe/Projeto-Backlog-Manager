/* ══════════════════════════════════════════════════════════════════
   catalogo.h  —  Lista encadeada dupla: fonte da verdade do sistema
   ══════════════════════════════════════════════════════════════════ */
#ifndef CATALOGO_H
#define CATALOGO_H

#include "tipos.h"

/* ─── Estado global do catálogo ─────────────────────────────────── */
extern NoJogo *catalogo_head;   /* primeiro nó da lista             */
extern NoJogo *catalogo_tail;   /* último nó (facilita append)      */
extern int     catalogo_total;  /* contagem de jogos cadastrados     */
extern int     proximo_id;      /* auto-incremento de ID (RN09)     */

/* ─── Ciclo de vida ─────────────────────────────────────────────── */
void    catalogo_inicializar(void);

/* ─── CRUD ──────────────────────────────────────────────────────── */
NoJogo *catalogo_inserir(DadosJogo dados);
int     catalogo_remover(int id);           /* retorna 1=ok, 0=não encontrado */
NoJogo *catalogo_buscar_id(int id);
int     catalogo_editar(int id, DadosJogo novo_dados);

/* ─── Busca por substring (RF05) ────────────────────────────────── */
/* Preenche resultados[0..n-1] com ponteiros para nós encontrados.  */
/* Retorna a quantidade de resultados.                               */
int     catalogo_buscar_nome(const char *termo,
                             NoJogo **resultados, int max_resultados);

/* ─── Ordenação (RF08) ──────────────────────────────────────────── */
void    catalogo_ordenar(int criterio);   /* ORDEM_ALFA / NOTA / STATUS */

/* ─── Utilitários ───────────────────────────────────────────────── */
void    catalogo_listar(void);            /* exibe tabela simples no terminal */
int     catalogo_vazio(void);

#endif /* CATALOGO_H */
