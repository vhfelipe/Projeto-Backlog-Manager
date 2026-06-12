/* ══════════════════════════════════════════════════════════════════
   backlog.c  —  Filas FIFO: "A Jogar" e "A Zerar"

   REGRA (RN04): enqueue sempre no tail, dequeue sempre no head.
   Os nós da fila guardam apenas Jogo* (referência ao catálogo).
   ══════════════════════════════════════════════════════════════════ */
#include "backlog.h"

/* ─── Nomes das filas para exibição ─────────────────────────────── */
static const char *NOMES_FILA[2] = { "A Jogar", "A Zerar" };

/* ─── Estado global ─────────────────────────────────────────────── */
Fila backlog_filas[2];

/* ══════════════════════════════════════════════════════════════════ */
void backlog_inicializar(void) {
    for (int i = 0; i < 2; i++) {
        backlog_filas[i].head     = NULL;
        backlog_filas[i].tail     = NULL;
        backlog_filas[i].tamanho  = 0;
    }
}

/* ══════════════════════════════════════════════════════════════════
   backlog_enqueue — insere referência no TAIL da fila (RN04).
   Retorna 1 se sucesso, 0 se já estiver na fila.
   ══════════════════════════════════════════════════════════════════ */
int backlog_enqueue(int fila_idx, NoJogo *ref) {
    if (fila_idx < 0 || fila_idx > 1 || !ref) return 0;
    Fila *f = &backlog_filas[fila_idx];

    /* Verifica duplicata */
    NoFila *cur = f->head;
    while (cur) {
        if (cur->ref == ref) return 0;  /* já está na fila */
        cur = cur->proximo;
    }

    NoFila *novo = (NoFila *)malloc(sizeof(NoFila));
    if (!novo) { perror("malloc enqueue"); return 0; }
    novo->ref     = ref;
    novo->proximo = NULL;

    if (!f->head) {
        f->head = novo;
        f->tail = novo;
    } else {
        f->tail->proximo = novo;
        f->tail          = novo;
    }
    f->tamanho++;
    return 1;
}

/* ══════════════════════════════════════════════════════════════════
   backlog_dequeue — remove e retorna o HEAD (RN04).
   Retorna NULL se a fila estiver vazia.
   ══════════════════════════════════════════════════════════════════ */
NoJogo *backlog_dequeue(int fila_idx) {
    if (fila_idx < 0 || fila_idx > 1) return NULL;
    Fila *f = &backlog_filas[fila_idx];
    if (!f->head) return NULL;

    NoFila *removido = f->head;
    NoJogo *ref      = removido->ref;

    f->head = removido->proximo;
    if (!f->head) f->tail = NULL;

    free(removido);
    f->tamanho--;
    return ref;
}

/* ══════════════════════════════════════════════════════════════════
   backlog_peek — consulta o head sem remover (RF11)
   ══════════════════════════════════════════════════════════════════ */
NoJogo *backlog_peek(int fila_idx) {
    if (fila_idx < 0 || fila_idx > 1) return NULL;
    Fila *f = &backlog_filas[fila_idx];
    if (!f->head) return NULL;
    return f->head->ref;
}

/* ══════════════════════════════════════════════════════════════════
   backlog_remover_referencia — remove da fila qualquer nó que
   aponte para 'ref'. Chamado por catalogo_remover() antes do free().
   ══════════════════════════════════════════════════════════════════ */
void backlog_remover_referencia(NoJogo *ref) {
    for (int i = 0; i < 2; i++) {
        Fila *f = &backlog_filas[i];
        NoFila *cur  = f->head;
        NoFila *prev = NULL;
        while (cur) {
            if (cur->ref == ref) {
                if (prev) prev->proximo    = cur->proximo;
                else      f->head          = cur->proximo;
                if (!cur->proximo)         f->tail = prev;
                NoFila *prox = cur->proximo;
                free(cur);
                f->tamanho--;
                cur = prox;
            } else {
                prev = cur;
                cur  = cur->proximo;
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════════════
   backlog_exibir_fila — exibe estado completo de uma fila (peek) (RF11)
   ══════════════════════════════════════════════════════════════════ */
void backlog_exibir_fila(int fila_idx) {
    if (fila_idx < 0 || fila_idx > 1) return;
    Fila *f = &backlog_filas[fila_idx];

    printf("\n  Fila \"%s\" (%d jogo(s)):\n",
           NOMES_FILA[fila_idx], f->tamanho);
    if (!f->head) {
        printf("    (fila vazia)\n");
        return;
    }
    printf("    HEAD\n");
    int pos = 1;
    NoFila *cur = f->head;
    while (cur) {
        printf("    [%d] ID:%-4d | %-30s | %s\n",
               pos++,
               cur->ref->dados.id,
               cur->ref->dados.nome,
               cur->ref->dados.plataforma);
        cur = cur->proximo;
    }
    printf("    TAIL\n");
}

/* ══════════════════════════════════════════════════════════════════
   backlog_menu_interativo — sub-menu de gerenciamento do backlog
   ══════════════════════════════════════════════════════════════════ */
void backlog_menu_interativo(void) {
    /* Importações locais para evitar dependência circular no .h */
    extern NoJogo *catalogo_buscar_id(int);
    extern void    catalogo_listar(void);

    int opcao, id, fila_idx;
    do {
        printf("\n  ╔══════════════════════════════╗\n");
        printf("  ║   GERENCIAR BACKLOG (FILAS)  ║\n");
        printf("  ╠══════════════════════════════╣\n");
        printf("  ║ [1] Enfileirar jogo          ║\n");
        printf("  ║ [2] Iniciar jogo (dequeue)   ║\n");
        printf("  ║ [3] Ver fila \"A Jogar\"       ║\n");
        printf("  ║ [4] Ver fila \"A Zerar\"       ║\n");
        printf("  ║ [0] Voltar                   ║\n");
        printf("  ╚══════════════════════════════╝\n");
        printf("  Opcao: ");

        if (scanf("%d", &opcao) != 1) { while(getchar()!='\n'); opcao=-1; }
        while (getchar() != '\n');

        switch (opcao) {
            case 1:
                catalogo_listar();
                printf("  ID do jogo: ");
                if (scanf("%d", &id) != 1) {
                    while(getchar()!='\n');
                    break;
                }
                while(getchar()!='\n');
                printf("  Fila (0=A Jogar, 1=A Zerar): ");
                if (scanf("%d", &fila_idx) != 1) {
                    while(getchar()!='\n');
                    break;
                }
                do { } while(getchar()!='\n');
                {
                    NoJogo *no = catalogo_buscar_id(id);
                    if (!no) { printf("  Jogo nao encontrado.\n"); break; }
                    if (backlog_enqueue(fila_idx, no))
                        printf("  \"%s\" adicionado a fila \"%s\".\n",
                               no->dados.nome, (fila_idx==0?"A Jogar":"A Zerar"));
                    else
                        printf("  Jogo ja esta na fila.\n");
                }
                break;
            case 2:
                printf("  Fila (0=A Jogar, 1=A Zerar): ");
                if (scanf("%d", &fila_idx) != 1) {
                    while(getchar()!='\n');
                    break;
                }
                do { } while(getchar()!='\n');
                {
                    NoJogo *no = backlog_dequeue(fila_idx);
                    if (!no) { printf("  Fila vazia.\n"); break; }
                    printf("  Iniciando: [%d] %s\n", no->dados.id, no->dados.nome);
                    char status_ant[MAX_STATUS];
                    strncpy(status_ant, no->dados.status, MAX_STATUS-1);
                    strncpy(no->dados.status, "Jogando", MAX_STATUS-1);
                    extern void historico_push_log(NoJogo*, const char*, const char*);
                    historico_push_log(no, status_ant, "Jogando");
                    printf("  Status atualizado para \"Jogando\".\n");
                }
                break;
            case 3:
                backlog_exibir_fila(FILA_A_JOGAR);
                break;
            case 4:
                backlog_exibir_fila(FILA_A_ZERAR);
                break;
            case 0:
                break;
            default:
                printf("  Opcao invalida.\n");
        }
    } while (opcao != 0);
}

/* ══════════════════════════════════════════════════════════════════ */
void backlog_liberar_tudo(void) {
    for (int i = 0; i < 2; i++) {
        NoFila *cur = backlog_filas[i].head;
        while (cur) {
            NoFila *prox = cur->proximo;
            free(cur);
            cur = prox;
        }
        backlog_filas[i].head    = NULL;
        backlog_filas[i].tail    = NULL;
        backlog_filas[i].tamanho = 0;
    }
}
