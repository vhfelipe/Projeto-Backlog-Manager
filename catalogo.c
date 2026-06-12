/* ══════════════════════════════════════════════════════════════════
   catalogo.c  —  Lista encadeada dupla (fonte da verdade)
   
   REGRAS CRÍTICAS (do documento de escopo):
     • Único módulo que chama malloc(sizeof(NoJogo))
     • Remoção: avisa carrossel, backlog e histórico ANTES do free()
     • Ordenação: reencadeia ponteiros, nunca cria novos nós
   ══════════════════════════════════════════════════════════════════ */
#include "catalogo.h"
#include "carrossel.h"
#include "backlog.h"
#include "historico.h"

/* ─── Estado global ─────────────────────────────────────────────── */
NoJogo *catalogo_head  = NULL;
NoJogo *catalogo_tail  = NULL;
int     catalogo_total = 0;
int     proximo_id     = 1;

/* ══════════════════════════════════════════════════════════════════ */
void catalogo_inicializar(void) {
    catalogo_head  = NULL;
    catalogo_tail  = NULL;
    catalogo_total = 0;
    proximo_id     = 1;
}

/* ══════════════════════════════════════════════════════════════════
   catalogo_inserir — aloca NoJogo, insere no final da lista dupla,
   atribui ID auto-incremento (RN09), retorna ponteiro para o nó.
   ══════════════════════════════════════════════════════════════════ */
NoJogo *catalogo_inserir(DadosJogo dados) {
    NoJogo *novo = (NoJogo *)malloc(sizeof(NoJogo));
    if (!novo) { perror("malloc catalogo_inserir"); return NULL; }

    /* Atribui ID único se ainda não definido (carga do arquivo usa id > 0) */
    if (dados.id <= 0) {
        dados.id = proximo_id;
    }
    /* Mantém proximo_id sempre maior que qualquer ID existente */
    if (dados.id >= proximo_id) {
        proximo_id = dados.id + 1;
    }

    novo->dados    = dados;
    novo->proximo  = NULL;
    novo->anterior = catalogo_tail;

    if (!catalogo_head) {
        catalogo_head = novo;
        catalogo_tail = novo;
    } else {
        catalogo_tail->proximo = novo;
        catalogo_tail          = novo;
    }
    catalogo_total++;
    return novo;
}

/* ══════════════════════════════════════════════════════════════════
   catalogo_remover — encontra o nó pelo ID, avisa todas as outras
   estruturas sobre a remoção iminente, desencadeia e libera.
   Retorna 1 se removeu, 0 se não encontrou.
   ══════════════════════════════════════════════════════════════════ */
int catalogo_remover(int id) {
    NoJogo *no = catalogo_buscar_id(id);
    if (!no) return 0;

    /* ── 1. Avisa demais estruturas (RN02 — integridade referencial) */
    carrossel_remover_referencia(no);
    backlog_remover_referencia(no);
    historico_remover_referencia(no);

    /* ── 2. Empilha cópia para undo ANTES de desconectar ─────────── */
    historico_push_undo(no);

    /* ── 3. Desencadeia da lista dupla ───────────────────────────── */
    if (no->anterior) no->anterior->proximo = no->proximo;
    else              catalogo_head         = no->proximo;

    if (no->proximo) no->proximo->anterior = no->anterior;
    else             catalogo_tail         = no->anterior;

    /* ── 4. Libera o nó — único free() de NoJogo no sistema ──────── */
    free(no);
    catalogo_total--;
    return 1;
}

/* ══════════════════════════════════════════════════════════════════ */
NoJogo *catalogo_buscar_id(int id) {
    NoJogo *cur = catalogo_head;
    while (cur) {
        if (cur->dados.id == id) return cur;
        cur = cur->proximo;
    }
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════
   catalogo_editar — atualiza os dados de um jogo existente.
   Se a plataforma mudou, avisa o carrossel para migrar (RN07).
   Se o status mudou, o histórico de log é registrado (RN10) pelo
   chamador (menu.c), pois envolve lógica de UI de confirmação.
   Retorna 1 se editou, 0 se não encontrou.
   ══════════════════════════════════════════════════════════════════ */
int catalogo_editar(int id, DadosJogo novo_dados) {
    NoJogo *no = catalogo_buscar_id(id);
    if (!no) return 0;

    int plataforma_mudou = (strcmp(no->dados.plataforma,
                                   novo_dados.plataforma) != 0);

    /* Preserva o ID original */
    novo_dados.id = no->dados.id;
    no->dados     = novo_dados;

    /* Migração de carrossel (RN07) */
    if (plataforma_mudou) {
        carrossel_migrar(no);
    }
    return 1;
}

/* ══════════════════════════════════════════════════════════════════
   catalogo_buscar_nome — travessia linear com strstr (RF05).
   Preenche vetor de ponteiros resultados[]. Retorna quantidade.
   ══════════════════════════════════════════════════════════════════ */
int catalogo_buscar_nome(const char *termo,
                         NoJogo **resultados, int max_resultados) {
    int count = 0;
    NoJogo *cur = catalogo_head;

    /* Cópia lower-case do termo para busca case-insensitive */
    char termo_low[MAX_NOME];
    int i = 0;
    while (termo[i] && i < MAX_NOME - 1) {
        termo_low[i] = (char)tolower((unsigned char)termo[i]);
        i++;
    }
    termo_low[i] = '\0';

    while (cur && count < max_resultados) {
        char nome_low[MAX_NOME];
        int j = 0;
        while (cur->dados.nome[j] && j < MAX_NOME - 1) {
            nome_low[j] = (char)tolower((unsigned char)cur->dados.nome[j]);
            j++;
        }
        nome_low[j] = '\0';

        if (strstr(nome_low, termo_low)) {
            resultados[count++] = cur;
        }
        cur = cur->proximo;
    }
    return count;
}

/* ══════════════════════════════════════════════════════════════════
   catalogo_ordenar — Insertion Sort reencadeando ponteiros (RF08).
   Não aloca novos nós; apenas reordena next/prev dos existentes.
   ══════════════════════════════════════════════════════════════════ */

/* Compara dois nós segundo o critério dado */
static int _comparar(const NoJogo *a, const NoJogo *b, int criterio) {
    switch (criterio) {
        case ORDEM_ALFA:
            return strcmp(a->dados.nome, b->dados.nome);
        case ORDEM_NOTA:
            /* Decrescente: nota maior primeiro */
            if (b->dados.nota > a->dados.nota) return  1;
            if (b->dados.nota < a->dados.nota) return -1;
            return 0;
        case ORDEM_STATUS:
            return strcmp(a->dados.status, b->dados.status);
        case ORDEM_ID:
            return a->dados.id - b->dados.id;
        default:
            return 0;
    }
}

void catalogo_ordenar(int criterio) {
    if (!catalogo_head || !catalogo_head->proximo) return;

    /* Insertion sort na lista dupla:
       Percorre da posição 1 em diante, move o nó para a posição
       correta desencadeando e reencadeando ponteiros. */
    NoJogo *i = catalogo_head->proximo;
    while (i) {
        NoJogo *proximo_i = i->proximo;

        /* Remove i da lista temporariamente */
        if (i->anterior) i->anterior->proximo = i->proximo;
        else             catalogo_head         = i->proximo;
        if (i->proximo) i->proximo->anterior  = i->anterior;
        else            catalogo_tail          = i->anterior;

        /* Encontra onde reencaixar */
        NoJogo *j = catalogo_head;
        while (j && _comparar(j, i, criterio) < 0) {
            j = j->proximo;
        }

        /* Insere i antes de j */
        if (!j) {
            /* Vai para o final */
            i->anterior = catalogo_tail;
            i->proximo  = NULL;
            if (catalogo_tail) catalogo_tail->proximo = i;
            else               catalogo_head           = i;
            catalogo_tail = i;
        } else {
            i->proximo  = j;
            i->anterior = j->anterior;
            if (j->anterior) j->anterior->proximo = i;
            else             catalogo_head         = i;
            j->anterior = i;
        }
        i = proximo_i;
    }
}

/* ══════════════════════════════════════════════════════════════════
   catalogo_listar — exibe tabela simples no terminal
   ══════════════════════════════════════════════════════════════════ */
void catalogo_listar(void) {
    if (catalogo_vazio()) {
        printf("  (catalogo vazio)\n");
        return;
    }
    printf("\n  %-4s %-30s %-14s %-10s %-5s %s\n",
           "ID", "Nome", "Plataforma", "Status", "Nota", "Ano");
    printf("  %s\n",
           "------------------------------------------------------------"
           "----------------");
    NoJogo *cur = catalogo_head;
    while (cur) {
        printf("  %-4d %-30s %-14s %-10s %-5.1f %d\n",
               cur->dados.id,
               cur->dados.nome,
               cur->dados.plataforma,
               cur->dados.status,
               cur->dados.nota,
               cur->dados.ano_lancamento);
        cur = cur->proximo;
    }
    printf("\n  Total: %d jogo(s)\n", catalogo_total);
}

int catalogo_vazio(void) {
    return catalogo_head == NULL;
}
