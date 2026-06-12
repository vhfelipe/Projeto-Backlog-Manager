/* ══════════════════════════════════════════════════════════════════
   carrossel.c  —  Listas circulares duplamente encadeadas c/ sentinela

   ESTRUTURA DE CADA CARROSSEL:
     sentinela ⇄ no1 ⇄ no2 ⇄ ... ⇄ noN ⇄ (volta ao sentinela)
   
   O sentinela possui ref == NULL e serve de âncora.
   A navegação salta o sentinela e exibe apenas nós com ref != NULL.
   ══════════════════════════════════════════════════════════════════ */
#include "carrossel.h"

#ifdef _WIN32
  #define LIMPAR_TELA "cls"
#else
  #define LIMPAR_TELA "clear"
#endif

/* ─── Estado global ─────────────────────────────────────────────── */
Carrossel *carrossel_lista = NULL;

/* ─── Auxiliares internas ───────────────────────────────────────── */
static Carrossel *_criar_carrossel(const char *plataforma);
static NoCircular *_criar_no_circular(NoJogo *ref);
static void _inserir_no_circular(Carrossel *c, NoJogo *ref);
static void _remover_no_circular(Carrossel *c, NoJogo *ref);
static void _liberar_carrossel(Carrossel *c);

/* ══════════════════════════════════════════════════════════════════ */
void carrossel_inicializar(void) {
    carrossel_lista = NULL;
    /* O carrossel GLOBAL é criado ao primeiro insert */
}

/* ─── Cria um novo carrossel com sentinela ──────────────────────── */
static Carrossel *_criar_carrossel(const char *plataforma) {
    Carrossel *c = (Carrossel *)malloc(sizeof(Carrossel));
    if (!c) { perror("malloc carrossel"); return NULL; }

    strncpy(c->plataforma, plataforma, MAX_PLATAFORMA - 1);
    c->plataforma[MAX_PLATAFORMA - 1] = '\0';
    c->tamanho = 0;
    c->proximo = NULL;

    /* Aloca o nó sentinela (ref = NULL marca que é sentinela — RN03) */
    NoCircular *s = (NoCircular *)malloc(sizeof(NoCircular));
    if (!s) { free(c); perror("malloc sentinela"); return NULL; }
    s->ref      = NULL;
    s->proximo  = s;   /* aponta para si mesmo: lista circular vazia */
    s->anterior = s;
    c->sentinela = s;

    return c;
}

/* ── Obtém (ou cria) o carrossel de uma plataforma ──────────────── */
Carrossel *carrossel_obter(const char *plataforma) {
    Carrossel *cur = carrossel_lista;
    while (cur) {
        if (strcmp(cur->plataforma, plataforma) == 0) return cur;
        cur = cur->proximo;
    }
    /* Não existe: cria e insere na cabeça da lista de carrosseis */
    Carrossel *novo = _criar_carrossel(plataforma);
    if (!novo) return NULL;
    novo->proximo  = carrossel_lista;
    carrossel_lista = novo;
    return novo;
}

/* ─── Cria nó de referência ─────────────────────────────────────── */
static NoCircular *_criar_no_circular(NoJogo *ref) {
    NoCircular *n = (NoCircular *)malloc(sizeof(NoCircular));
    if (!n) { perror("malloc no circular"); return NULL; }
    n->ref      = ref;
    n->proximo  = NULL;
    n->anterior = NULL;
    return n;
}

/* ─── Insere nó ANTES do sentinela (equivale ao final da lista) ─── */
static void _inserir_no_circular(Carrossel *c, NoJogo *ref) {
    NoCircular *novo = _criar_no_circular(ref);
    if (!novo) return;

    NoCircular *s    = c->sentinela;
    NoCircular *ant  = s->anterior;

    novo->proximo  = s;
    novo->anterior = ant;
    ant->proximo   = novo;
    s->anterior    = novo;

    c->tamanho++;
}

/* ─── Remove nó que referencia 'ref' do carrossel ───────────────── */
static void _remover_no_circular(Carrossel *c, NoJogo *ref) {
    NoCircular *cur = c->sentinela->proximo;
    while (cur != c->sentinela) {
        if (cur->ref == ref) {
            cur->anterior->proximo = cur->proximo;
            cur->proximo->anterior = cur->anterior;
            free(cur);
            c->tamanho--;
            return;
        }
        cur = cur->proximo;
    }
}

/* ══════════════════════════════════════════════════════════════════
   carrossel_inserir — insere referência no carrossel GLOBAL e no
   carrossel da plataforma do jogo.
   ══════════════════════════════════════════════════════════════════ */
void carrossel_inserir(NoJogo *ref) {
    if (!ref) return;

    /* Carrossel global */
    Carrossel *global = carrossel_obter("GLOBAL");
    _inserir_no_circular(global, ref);

    /* Carrossel da plataforma */
    Carrossel *plat = carrossel_obter(ref->dados.plataforma);
    _inserir_no_circular(plat, ref);
}

/* ══════════════════════════════════════════════════════════════════
   carrossel_remover_referencia — remove de TODOS os carrosseis
   (chamado por catalogo_remover antes do free do nó — RN02)
   ══════════════════════════════════════════════════════════════════ */
void carrossel_remover_referencia(NoJogo *ref) {
    if (!ref) return;
    Carrossel *cur = carrossel_lista;
    while (cur) {
        _remover_no_circular(cur, ref);
        cur = cur->proximo;
    }
}

/* ══════════════════════════════════════════════════════════════════
   carrossel_migrar — troca de plataforma (RN07):
   remove do carrossel antigo e insere no novo (GLOBAL permanece).
   ══════════════════════════════════════════════════════════════════ */
void carrossel_migrar(NoJogo *ref) {
    if (!ref) return;

    /* Remove de todos os carrosseis de plataforma (não o GLOBAL) */
    Carrossel *cur = carrossel_lista;
    while (cur) {
        if (strcmp(cur->plataforma, "GLOBAL") != 0) {
            _remover_no_circular(cur, ref);
        }
        cur = cur->proximo;
    }

    /* Insere no carrossel da nova plataforma */
    Carrossel *nova = carrossel_obter(ref->dados.plataforma);
    _inserir_no_circular(nova, ref);
}

/* ══════════════════════════════════════════════════════════════════
   carrossel_navegar — loop interativo de navegação (RF06 / RF07)
   ══════════════════════════════════════════════════════════════════ */
void carrossel_navegar(const char *plataforma) {
    Carrossel *c = carrossel_obter(plataforma);
    if (!c || c->tamanho == 0) {
        printf("\n  [ Carrossel \"%s\" esta vazio ]\n", plataforma);
        printf("  Pressione ENTER para voltar...");
        getchar();
        return;
    }

    /* Começa no primeiro nó real (pula o sentinela) */
    NoCircular *cursor = c->sentinela->proximo;
    char entrada;

    while (1) {
        /* Pula sentinela se navegação circular chegou nele */
        if (!cursor->ref) cursor = cursor->proximo;
        if (!cursor->ref) break; /* lista ficou vazia */

        system(LIMPAR_TELA);
        printf("\n");
        printf("  ╔══════════════════════════════════════════════╗\n");
        printf("  ║  CARROSSEL: %-32s║\n", plataforma);
        printf("  ╠══════════════════════════════════════════════╣\n");

        DadosJogo *d = &cursor->ref->dados;
        printf("  ║                                              ║\n");
        printf("  ║  ID       : %-32d║\n", d->id);
        printf("  ║  Nome     : %-32s║\n", d->nome);
        printf("  ║  Plataforma: %-31s║\n", d->plataforma);
        printf("  ║  Genero   : %-32s║\n", d->genero);
        printf("  ║  Ano      : %-32d║\n", d->ano_lancamento);
        printf("  ║  Status   : %-32s║\n", d->status);
        printf("  ║  Nota     : %-32.1f║\n", d->nota);
        printf("  ║                                              ║\n");
        printf("  ╠══════════════════════════════════════════════╣\n");
        printf("  ║  < A = Anterior   D = Proximo   Q = Sair >  ║\n");
        printf("  ╚══════════════════════════════════════════════╝\n");
        printf("  Opcao: ");

        if (scanf(" %c", &entrada) != 1) { while(getchar()!='\n'); continue; }
        while(getchar() != '\n');

        entrada = (char)tolower((unsigned char)entrada);
        if (entrada == 'd') {
            cursor = cursor->proximo;
            if (!cursor->ref) cursor = cursor->proximo; /* pula sentinela */
        } else if (entrada == 'a') {
            cursor = cursor->anterior;
            if (!cursor->ref) cursor = cursor->anterior; /* pula sentinela */
        } else if (entrada == 'q') {
            break;
        }
    }
}

/* ══════════════════════════════════════════════════════════════════
   carrossel_listar_plataformas — exibe as plataformas disponíveis
   ══════════════════════════════════════════════════════════════════ */
void carrossel_listar_plataformas(void) {
    printf("\n  Carrosseis disponíveis:\n");
    Carrossel *cur = carrossel_lista;
    int i = 1;
    while (cur) {
        printf("    [%d] %s (%d jogo(s))\n", i++, cur->plataforma, cur->tamanho);
        cur = cur->proximo;
    }
}

/* ══════════════════════════════════════════════════════════════════
   Libera nós de um carrossel (sentinela + todos os nós de referência)
   ══════════════════════════════════════════════════════════════════ */
static void _liberar_carrossel(Carrossel *c) {
    NoCircular *cur = c->sentinela->proximo;
    while (cur != c->sentinela) {
        NoCircular *prox = cur->proximo;
        free(cur);
        cur = prox;
    }
    free(c->sentinela);
    free(c);
}

void carrossel_liberar_tudo(void) {
    Carrossel *cur = carrossel_lista;
    while (cur) {
        Carrossel *prox = cur->proximo;
        _liberar_carrossel(cur);
        cur = prox;
    }
    carrossel_lista = NULL;
}
