/* ══════════════════════════════════════════════════════════════════
   historico.c  —  Pilhas LIFO: undo, acessos, conquistas, log

   REGRA (RN05): push empilha no topo; pop retira do topo.
   pilha_undo: guarda CÓPIA dos dados (NoJogo já será liberado).
   demais pilhas: guardam ponteiros para o catálogo.
   ══════════════════════════════════════════════════════════════════ */
#include "historico.h"
#include "catalogo.h"
#include "carrossel.h"

/* ─── Estado global ─────────────────────────────────────────────── */
NoPilhaUndo *pilha_undo       = NULL;
NoPilhaRef  *pilha_acessos    = NULL;
NoPilhaRef  *pilha_zerados    = NULL;
NoPilhaRef  *pilha_platinados = NULL;
NoPilhaLog  *pilha_log_status = NULL;

/* ─── Limite de acessos rastreados ──────────────────────────────── */
#define MAX_ACESSOS 10

/* ══════════════════════════════════════════════════════════════════ */
void historico_inicializar(void) {
    pilha_undo       = NULL;
    pilha_acessos    = NULL;
    pilha_zerados    = NULL;
    pilha_platinados = NULL;
    pilha_log_status = NULL;
}

/* ══════════════════════════════════════════════════════════════════
   historico_push_undo — guarda CÓPIA dos dados do jogo que será
   deletado. Chamado ANTES do free() em catalogo_remover().
   ══════════════════════════════════════════════════════════════════ */
void historico_push_undo(NoJogo *ref) {
    if (!ref) return;
    NoPilhaUndo *novo = (NoPilhaUndo *)malloc(sizeof(NoPilhaUndo));
    if (!novo) { perror("malloc push_undo"); return; }
    novo->dados    = ref->dados;    /* cópia completa da struct DadosJogo */
    novo->proximo  = pilha_undo;
    pilha_undo     = novo;
}

/* ══════════════════════════════════════════════════════════════════
   historico_pop_undo — restaura o último jogo deletado.
   Recria o nó na lista encadeada e nos carrosseis.
   Retorna 1 se restaurou, 0 se pilha vazia.
   ══════════════════════════════════════════════════════════════════ */
int historico_pop_undo(void) {
    if (!pilha_undo) {
        printf("  Nenhuma exclusao para desfazer.\n");
        return 0;
    }
    NoPilhaUndo *topo = pilha_undo;
    pilha_undo        = topo->proximo;

    printf("  Restaurando: [%d] %s\n", topo->dados.id, topo->dados.nome);

    /* Recria o nó no catálogo */
    NoJogo *restaurado = catalogo_inserir(topo->dados);
    if (restaurado) {
        /* Reinsere nos carrosseis */
        carrossel_inserir(restaurado);
        printf("  Jogo restaurado com sucesso!\n");
    }

    free(topo);
    return 1;
}

/* ══════════════════════════════════════════════════════════════════
   historico_push_acesso — empilha nos últimos acessos (max 10).
   ══════════════════════════════════════════════════════════════════ */
void historico_push_acesso(NoJogo *ref) {
    if (!ref) return;

    /* Conta itens e remove o mais antigo se ultrapassar o limite */
    int count = 0;
    NoPilhaRef *cur = pilha_acessos;
    while (cur) { count++; cur = cur->proximo; }

    if (count >= MAX_ACESSOS) {
        /* Remove o último (mais antigo) da pilha */
        NoPilhaRef *prev = NULL;
        cur = pilha_acessos;
        while (cur->proximo) { prev = cur; cur = cur->proximo; }
        if (prev) prev->proximo = NULL;
        else      pilha_acessos = NULL;
        free(cur);
    }

    NoPilhaRef *novo = (NoPilhaRef *)malloc(sizeof(NoPilhaRef));
    if (!novo) { perror("malloc push_acesso"); return; }
    novo->ref     = ref;
    novo->proximo = pilha_acessos;
    pilha_acessos = novo;
}

/* ══════════════════════════════════════════════════════════════════
   historico_push_conquista — empilha no topo da pilha correta
   baseando-se no status atual do jogo (RF12, RN10).
   ══════════════════════════════════════════════════════════════════ */
void historico_push_conquista(NoJogo *ref) {
    if (!ref) return;

    /* Seleciona a pilha correta antes de qualquer alocacao */
    NoPilhaRef **pilha = (strcmp(ref->dados.status, "Platinado") == 0)
                         ? &pilha_platinados : &pilha_zerados;

    /* Verificacao de duplicata: percorre a pilha inteira (RF: sem repeticoes) */
    NoPilhaRef *cur = *pilha;
    while (cur) {
        if (cur->ref == ref) return;   /* jogo ja registrado — ignora silenciosamente */
        cur = cur->proximo;
    }

    NoPilhaRef *novo = (NoPilhaRef *)malloc(sizeof(NoPilhaRef));
    if (!novo) { perror("malloc push_conquista"); return; }
    novo->ref     = ref;
    novo->proximo = *pilha;
    *pilha        = novo;
}

/* ══════════════════════════════════════════════════════════════════
   historico_push_log — registra transição de status (RN10).
   ══════════════════════════════════════════════════════════════════ */
void historico_push_log(NoJogo *ref, const char *ant, const char *novo_s) {
    if (!ref) return;
    NoPilhaLog *no = (NoPilhaLog *)malloc(sizeof(NoPilhaLog));
    if (!no) { perror("malloc push_log"); return; }

    no->ref = ref;
    strncpy(no->status_anterior, ant,   MAX_STATUS - 1);
    strncpy(no->status_novo,     novo_s, MAX_STATUS - 1);
    no->status_anterior[MAX_STATUS-1] = '\0';
    no->status_novo[MAX_STATUS-1]     = '\0';

    no->proximo      = pilha_log_status;
    pilha_log_status = no;
}

/* ══════════════════════════════════════════════════════════════════
   historico_remover_referencia — remove de todas as pilhas de REF
   (NÃO remove da pilha_undo, que tem cópias). Chamado antes do free.
   ══════════════════════════════════════════════════════════════════ */
static void _remover_ref_pilha(NoPilhaRef **topo, NoJogo *ref) {
    NoPilhaRef *cur  = *topo;
    NoPilhaRef *prev = NULL;
    while (cur) {
        if (cur->ref == ref) {
            if (prev) prev->proximo = cur->proximo;
            else      *topo         = cur->proximo;
            NoPilhaRef *prox = cur->proximo;
            free(cur);
            cur = prox;
        } else {
            prev = cur;
            cur  = cur->proximo;
        }
    }
}

static void _remover_log_pilha(NoPilhaLog **topo, NoJogo *ref) {
    NoPilhaLog *cur  = *topo;
    NoPilhaLog *prev = NULL;
    while (cur) {
        if (cur->ref == ref) {
            if (prev) prev->proximo = cur->proximo;
            else      *topo         = cur->proximo;
            NoPilhaLog *prox = cur->proximo;
            free(cur);
            cur = prox;
        } else {
            prev = cur;
            cur  = cur->proximo;
        }
    }
}

void historico_remover_referencia(NoJogo *ref) {
    _remover_ref_pilha(&pilha_acessos,    ref);
    _remover_ref_pilha(&pilha_zerados,    ref);
    _remover_ref_pilha(&pilha_platinados, ref);
    _remover_log_pilha(&pilha_log_status, ref);
    /* pilha_undo NÃO é tocada: tem cópia independente dos dados */
}

/* ══════════════════════════════════════════════════════════════════
   historico_menu_interativo — sub-menu de histórico / pilhas (RF14)
   ══════════════════════════════════════════════════════════════════ */
static void _exibir_pilha_ref(NoPilhaRef *topo, const char *titulo) {
    printf("\n  === %s ===\n", titulo);
    if (!topo) { printf("    (vazia)\n"); return; }
    int i = 1;
    NoPilhaRef *cur = topo;
    while (cur) {
        printf("    [%d] ID:%-4d | %-30s | %s\n",
               i++, cur->ref->dados.id,
               cur->ref->dados.nome,
               cur->ref->dados.status);
        cur = cur->proximo;
    }
}

static void _exibir_pilha_log(void) {
    printf("\n  === Log de Alteracoes de Status ===\n");
    if (!pilha_log_status) { printf("    (vazio)\n"); return; }
    int i = 1;
    NoPilhaLog *cur = pilha_log_status;
    while (cur) {
        printf("    [%d] ID:%-4d %-25s : %s  ->  %s\n",
               i++, cur->ref->dados.id,
               cur->ref->dados.nome,
               cur->status_anterior,
               cur->status_novo);
        cur = cur->proximo;
    }
}

static void _exibir_pilha_undo(void) {
    printf("\n  === Historico de Exclusoes (Undo) ===\n");
    if (!pilha_undo) { printf("    (vazia)\n"); return; }
    int i = 1;
    NoPilhaUndo *cur = pilha_undo;
    while (cur) {
        printf("    [%d] ID:%-4d | %-30s | %s\n",
               i++, cur->dados.id,
               cur->dados.nome,
               cur->dados.plataforma);
        cur = cur->proximo;
    }
}

void historico_menu_interativo(void) {
    int opcao;
    do {
        printf("\n  ╔════════════════════════════════╗\n");
        printf("  ║   HISTORICO E PILHAS           ║\n");
        printf("  ╠════════════════════════════════╣\n");
        printf("  ║ [1] Ultimos acessos            ║\n");
        printf("  ║ [2] Jogos Zerados              ║\n");
        printf("  ║ [3] Jogos Platinados           ║\n");
        printf("  ║ [4] Log de Status              ║\n");
        printf("  ║ [5] Ver exclusoes (undo)       ║\n");
        printf("  ║ [6] Desfazer ultima exclusao   ║\n");
        printf("  ║ [0] Voltar                     ║\n");
        printf("  ╚════════════════════════════════╝\n");
        printf("  Opcao: ");

        if (scanf("%d", &opcao) != 1) { while(getchar()!='\n'); opcao=-1; }
        while (getchar() != '\n');

        switch (opcao) {
            case 1: _exibir_pilha_ref(pilha_acessos,    "Ultimos Acessos");  break;
            case 2: _exibir_pilha_ref(pilha_zerados,    "Jogos Zerados");    break;
            case 3: _exibir_pilha_ref(pilha_platinados, "Jogos Platinados"); break;
            case 4: _exibir_pilha_log();                                      break;
            case 5: _exibir_pilha_undo();                                     break;
            case 6: historico_pop_undo();                                      break;
            case 0: break;
            default: printf("  Opcao invalida.\n");
        }
    } while (opcao != 0);
}

/* ══════════════════════════════════════════════════════════════════ */
void historico_liberar_tudo(void) {
    /* pilha_undo */
    while (pilha_undo) {
        NoPilhaUndo *p = pilha_undo->proximo;
        free(pilha_undo);
        pilha_undo = p;
    }
    /* pilhas de referência */
    while (pilha_acessos) {
        NoPilhaRef *p = pilha_acessos->proximo;
        free(pilha_acessos);
        pilha_acessos = p;
    }
    while (pilha_zerados) {
        NoPilhaRef *p = pilha_zerados->proximo;
        free(pilha_zerados);
        pilha_zerados = p;
    }
    while (pilha_platinados) {
        NoPilhaRef *p = pilha_platinados->proximo;
        free(pilha_platinados);
        pilha_platinados = p;
    }
    /* pilha de log */
    while (pilha_log_status) {
        NoPilhaLog *p = pilha_log_status->proximo;
        free(pilha_log_status);
        pilha_log_status = p;
    }
}
