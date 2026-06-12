/* ══════════════════════════════════════════════════════════════════
   tipos.h  —  Structs e constantes compartilhadas do Backlog Manager
   ══════════════════════════════════════════════════════════════════ */
#ifndef TIPOS_H
#define TIPOS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ─── Limites de campo ─────────────────────────────────────────── */
#define MAX_NOME        100
#define MAX_PLATAFORMA   50
#define MAX_GENERO       50
#define MAX_STATUS       30
#define ARQUIVO_DADOS   "biblioteca.bin"

/* ─── Critérios de ordenação ───────────────────────────────────── */
#define ORDEM_ALFA      1
#define ORDEM_NOTA      2
#define ORDEM_STATUS    3
#define ORDEM_ID        4

/* ─── Índices das filas ────────────────────────────────────────── */
#define FILA_A_JOGAR    0
#define FILA_A_ZERAR    1

/* ══════════════════════════════════════════════════════════════════
   DADOS SERIALIZÁVEIS — gravados e lidos do arquivo .bin
   Ponteiros NUNCA vão para o arquivo.
   ══════════════════════════════════════════════════════════════════ */
typedef struct {
    int   id;
    char  nome[MAX_NOME];
    char  plataforma[MAX_PLATAFORMA];
    char  genero[MAX_GENERO];
    char  status[MAX_STATUS];   /* "A Jogar" | "Jogando" | "Zerado" | "Platinado" */
    float nota;                 /* [0.0, 10.0]  (RN08) */
    int   ano_lancamento;
} DadosJogo;

/* ══════════════════════════════════════════════════════════════════
   NÓ DA LISTA ENCADEADA DUPLA — catálogo base (fonte da verdade)
   Único lugar onde malloc(sizeof(NoJogo)) é chamado no sistema.
   ══════════════════════════════════════════════════════════════════ */
typedef struct NoJogo {
    DadosJogo     dados;
    struct NoJogo *proximo;
    struct NoJogo *anterior;
} NoJogo;

/* ══════════════════════════════════════════════════════════════════
   NÓ DAS LISTAS CIRCULARES — guarda apenas ponteiro de referência
   ref == NULL → é o nó sentinela (RN03)
   ══════════════════════════════════════════════════════════════════ */
typedef struct NoCircular {
    NoJogo            *ref;
    struct NoCircular *proximo;
    struct NoCircular *anterior;
} NoCircular;

/* ══════════════════════════════════════════════════════════════════
   CARROSSEL — lista circular com sentinela
   Cada plataforma tem o seu; o primeiro da cadeia é sempre "GLOBAL".
   ══════════════════════════════════════════════════════════════════ */
typedef struct Carrossel {
    char              plataforma[MAX_PLATAFORMA];
    NoCircular       *sentinela;
    int               tamanho;
    struct Carrossel *proximo;   /* próximo carrossel na lista de carrosseis */
} Carrossel;

/* ══════════════════════════════════════════════════════════════════
   NÓ DA FILA (FIFO) — referência ao NoJogo do catálogo
   ══════════════════════════════════════════════════════════════════ */
typedef struct NoFila {
    NoJogo        *ref;
    struct NoFila *proximo;
} NoFila;

/* ─── Fila FIFO com head e tail ────────────────────────────────── */
typedef struct {
    NoFila *head;
    NoFila *tail;
    int     tamanho;
} Fila;

/* ══════════════════════════════════════════════════════════════════
   NÓ DA PILHA DE DESFAZER — guarda CÓPIA dos dados (não ponteiro)
   Necessário porque o NoJogo original é liberado com free() na remoção.
   ══════════════════════════════════════════════════════════════════ */
typedef struct NoPilhaUndo {
    DadosJogo           dados;   /* cópia completa, sem ponteiros */
    struct NoPilhaUndo *proximo;
} NoPilhaUndo;

/* ══════════════════════════════════════════════════════════════════
   NÓ DAS PILHAS DE REFERÊNCIA — conquistas e últimos acessos
   ══════════════════════════════════════════════════════════════════ */
typedef struct NoPilhaRef {
    NoJogo            *ref;
    struct NoPilhaRef *proximo;
} NoPilhaRef;

/* ══════════════════════════════════════════════════════════════════
   NÓ DA PILHA DE LOG DE STATUS — registra transições (RN10)
   ══════════════════════════════════════════════════════════════════ */
typedef struct NoPilhaLog {
    NoJogo            *ref;
    char               status_anterior[MAX_STATUS];
    char               status_novo[MAX_STATUS];
    struct NoPilhaLog *proximo;
} NoPilhaLog;

#endif /* TIPOS_H */
