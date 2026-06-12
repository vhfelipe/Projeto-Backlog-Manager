/* ══════════════════════════════════════════════════════════════════
   menu.c  —  Interface CLI: lê entradas, valida e roteia chamadas.
   Este módulo NÃO contém lógica de dados — apenas UI e roteamento.
   ══════════════════════════════════════════════════════════════════ */
#include "menu.h"
#include "catalogo.h"
#include "carrossel.h"
#include "backlog.h"
#include "historico.h"
#include "persistencia.h"
#include "plataformas.h"

#ifdef _WIN32
  #define LIMPAR_TELA "cls"
#else
  #define LIMPAR_TELA "clear"
#endif

/* ══════════════════════════════════════════════════════════════════
   Utilitários de entrada
   ══════════════════════════════════════════════════════════════════ */
void menu_limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void menu_ler_string(const char *prompt, char *destino, int tamanho) {
    printf("  %s", prompt);
    if (fgets(destino, tamanho, stdin)) {
        int len = (int)strlen(destino);
        if (len > 0 && destino[len - 1] == '\n')
            destino[len - 1] = '\0';
    }
}

int menu_ler_inteiro(const char *prompt) {
    int v;
    printf("  %s", prompt);
    if (scanf("%d", &v) != 1) { menu_limpar_buffer(); return -1; }
    menu_limpar_buffer();
    return v;
}

/* Lê e valida nota no intervalo [0.0, 10.0] (RN08) */
float menu_ler_nota(void) {
    float v;
    do {
        printf("  Nota (0.0 a 10.0): ");
        if (scanf("%f", &v) != 1) { menu_limpar_buffer(); v = -1; continue; }
        menu_limpar_buffer();
        if (v < 0.0f || v > 10.0f)
            printf("  Nota invalida. Insira um valor entre 0.0 e 10.0.\n");
    } while (v < 0.0f || v > 10.0f);
    return v;
}


/* ─── Status válidos ────────────────────────────────────────────── */
static const char *STATUS_VALIDOS[] = {
    "A Jogar", "Jogando", "Zerado", "Platinado"
};
#define N_STATUS 4

static void _menu_escolher_status(char *destino) {
    printf("\n  Status:\n");
    for (int i = 0; i < N_STATUS; i++)
        printf("    [%d] %s\n", i + 1, STATUS_VALIDOS[i]);
    int op;
    do {
        printf("  Escolha (1-%d): ", N_STATUS);
        if (scanf("%d", &op) != 1) { menu_limpar_buffer(); op = 0; }
        else menu_limpar_buffer();
    } while (op < 1 || op > N_STATUS);
    strncpy(destino, STATUS_VALIDOS[op - 1], MAX_STATUS - 1);
    destino[MAX_STATUS - 1] = '\0';
}

/* ══════════════════════════════════════════════════════════════════
   Ação 1 — Cadastrar Jogo (RF01)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_cadastrar(void) {
    printf("\n  ── CADASTRAR NOVO JOGO ──────────────────\n");
    DadosJogo d;
    memset(&d, 0, sizeof(d));
    d.id = 0;

    menu_ler_string("Nome       : ", d.nome, MAX_NOME);
    if (strlen(d.nome) == 0) {
        printf("  Nome obrigatorio. Cadastro cancelado.\n");
        return;
    }

    /* Plataforma com validação e normalização */
    printf("  (digite '?' para ver plataformas aceitas)\n");
    char raw_plat[MAX_PLATAFORMA];
    const char *canon = NULL;
    do {
        menu_ler_string("Plataforma : ", raw_plat, MAX_PLATAFORMA);
        if (strcmp(raw_plat, "?") == 0) {
            plataforma_listar_validas();
            printf("\n");
            continue;
        }
        canon = plataforma_normalizar(raw_plat);
        if (!canon) {
            printf("  Plataforma \"%s\" nao reconhecida. ", raw_plat);
            printf("Digite '?' para ver a lista.\n");
        }
    } while (!canon);
    strncpy(d.plataforma, canon, MAX_PLATAFORMA - 1);

    menu_ler_string("Genero     : ", d.genero, MAX_GENERO);

    printf("  Ano de lancamento: ");
    if (scanf("%d", &d.ano_lancamento) != 1) {
        menu_limpar_buffer();
        d.ano_lancamento = 0;
    } else {
        menu_limpar_buffer();
    }

    _menu_escolher_status(d.status);
    d.nota = menu_ler_nota();

    NoJogo *no = catalogo_inserir(d);
    if (no) {
        carrossel_inserir(no);
        /* Auto-registra conquista se o jogo ja foi cadastrado como Zerado/Platinado */
        if (strcmp(no->dados.status, "Zerado")    == 0 ||
            strcmp(no->dados.status, "Platinado") == 0) {
            historico_push_conquista(no);
            printf("  Conquista registrada automaticamente!\n");
        }
        printf("  Jogo \"%s\" cadastrado com ID %d na plataforma \"%s\"!\n",
               no->dados.nome, no->dados.id, no->dados.plataforma);
    }
}

/* ══════════════════════════════════════════════════════════════════
   Ação 2 — Remover Jogo (RF02)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_remover(void) {
    catalogo_listar();
    if (catalogo_vazio()) return;
    int id = menu_ler_inteiro("ID do jogo a remover: ");
    NoJogo *no = catalogo_buscar_id(id);
    if (!no) { printf("  Jogo ID %d nao encontrado.\n", id); return; }

    printf("  Confirmar remocao de \"%s\"? (s/n): ", no->dados.nome);
    char c; scanf(" %c", &c); menu_limpar_buffer();
    if (tolower((unsigned char)c) != 's') { printf("  Cancelado.\n"); return; }

    if (catalogo_remover(id))
        printf("  Jogo removido. Use Historico > Desfazer para restaurar.\n");
}

/* ══════════════════════════════════════════════════════════════════
   Ação 3 — Editar Jogo (RF03 / RF04)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_editar(void) {
    catalogo_listar();
    if (catalogo_vazio()) return;
    int id = menu_ler_inteiro("ID do jogo a editar: ");
    NoJogo *no = catalogo_buscar_id(id);
    if (!no) { printf("  Jogo ID %d nao encontrado.\n", id); return; }

    printf("\n  Editando: [%d] %s\n", no->dados.id, no->dados.nome);
    printf("  (ENTER = manter valor atual)\n\n");

    DadosJogo novo = no->dados;
    char buf[MAX_NOME];

    printf("  Nome atual     : %s\n", novo.nome);
    menu_ler_string("  Novo nome      : ", buf, MAX_NOME);
    if (strlen(buf) > 0) strncpy(novo.nome, buf, MAX_NOME - 1);

    /* Plataforma com normalização — Enter mantém a atual */
    printf("  Plataforma atual: %s\n", novo.plataforma);
    printf("  (ENTER = manter | '?' = listar plataformas aceitas)\n");
    char raw_plat[MAX_PLATAFORMA];
    const char *canon = NULL;
    int plat_alterada = 0;
    do {
        menu_ler_string("  Nova plataforma: ", raw_plat, MAX_PLATAFORMA);
        if (raw_plat[0] == '\0') break;         /* Enter → mantém */
        if (strcmp(raw_plat, "?") == 0) {
            plataforma_listar_validas(); printf("\n");
            continue;
        }
        canon = plataforma_normalizar(raw_plat);
        if (!canon) {
            printf("  Plataforma \"%s\" nao reconhecida. Digite '?' para ver a lista.\n",
                   raw_plat);
        } else {
            strncpy(novo.plataforma, canon, MAX_PLATAFORMA - 1);
            plat_alterada = 1;
        }
    } while (raw_plat[0] != '\0' && !canon);
    (void)plat_alterada;

    printf("  Genero atual   : %s\n", novo.genero);
    menu_ler_string("  Novo genero    : ", buf, MAX_GENERO);
    if (strlen(buf) > 0) strncpy(novo.genero, buf, MAX_GENERO - 1);

    printf("  Ano atual      : %d\n", novo.ano_lancamento);
    int ano = menu_ler_inteiro("  Novo ano (0=manter): ");
    if (ano > 0) novo.ano_lancamento = ano;

    printf("  Status atual   : %s\n", novo.status);
    printf("  Alterar status? (s/n): ");
    char c; scanf(" %c", &c); menu_limpar_buffer();
    char status_ant[MAX_STATUS];
    strncpy(status_ant, no->dados.status, MAX_STATUS - 1);
    if (tolower((unsigned char)c) == 's') {
        _menu_escolher_status(novo.status);
    }

    printf("  Nota atual     : %.1f\n", novo.nota);
    printf("  Alterar nota? (s/n): ");
    scanf(" %c", &c); menu_limpar_buffer();
    if (tolower((unsigned char)c) == 's')
        novo.nota = menu_ler_nota();

    catalogo_editar(id, novo);

    if (strcmp(status_ant, novo.status) != 0) {
        no = catalogo_buscar_id(id);
        if (no) {
            historico_push_log(no, status_ant, novo.status);
            if (strcmp(novo.status, "Zerado")    == 0 ||
                strcmp(novo.status, "Platinado") == 0) {
                historico_push_conquista(no);
            }
        }
    }
    printf("  Jogo atualizado!\n");
}

/* ══════════════════════════════════════════════════════════════════
   Ação 4 — Buscar Jogo (RF05)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_buscar(void) {
    char termo[MAX_NOME];
    menu_ler_string("Termo de busca: ", termo, MAX_NOME);
    if (strlen(termo) == 0) return;

    NoJogo *resultados[200];
    int n = catalogo_buscar_nome(termo, resultados, 200);

    if (n == 0) {
        printf("  Nenhum jogo encontrado para \"%s\".\n", termo);
        return;
    }
    printf("\n  %d resultado(s) para \"%s\":\n\n", n, termo);
    printf("  %-4s %-30s %-14s %-10s %s\n",
           "ID", "Nome", "Plataforma", "Status", "Nota");
    printf("  %s\n",
           "──────────────────────────────────────────────────────────────");
    for (int i = 0; i < n; i++) {
        DadosJogo *d = &resultados[i]->dados;
        printf("  %-4d %-30s %-14s %-10s %.1f\n",
               d->id, d->nome, d->plataforma, d->status, d->nota);
        historico_push_acesso(resultados[i]);
    }
}

/* ══════════════════════════════════════════════════════════════════
   Ação 5 — Ordenar Catálogo (RF08)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_ordenar(void) {
    printf("\n  Ordenar por:\n");
    printf("    [1] Ordem Alfabetica\n");
    printf("    [2] Nota (maior primeiro)\n");
    printf("    [3] Status\n");
    printf("    [4] ID (crescente)\n");
    int op = menu_ler_inteiro("Criterio: ");
    if (op < 1 || op > 4) { printf("  Opcao invalida.\n"); return; }
    catalogo_ordenar(op);
    printf("  Catalogo reordenado!\n");
    catalogo_listar();
}

/* ══════════════════════════════════════════════════════════════════
   Ação 6 — Navegar por Carrosseis (RF06 / RF07)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_carrossel(void) {
    printf("\n");
    carrossel_listar_plataformas();

    if (!carrossel_lista) {
        printf("  Nenhum carrossel disponivel.\n");
        return;
    }

    int total = 0;
    Carrossel *cur = carrossel_lista;
    while (cur) { total++; cur = cur->proximo; }

    int op = menu_ler_inteiro("Numero do carrossel (0=cancelar): ");
    if (op <= 0 || op > total) return;

    cur = carrossel_lista;
    for (int i = 1; i < op && cur; i++) cur = cur->proximo;
    if (cur) carrossel_navegar(cur->plataforma);
}

/* ══════════════════════════════════════════════════════════════════
   Ação 7 — Marcar Conquista (RF12)
   ══════════════════════════════════════════════════════════════════ */
static void _menu_conquista(void) {
    catalogo_listar();
    if (catalogo_vazio()) return;
    int id = menu_ler_inteiro("ID do jogo: ");
    NoJogo *no = catalogo_buscar_id(id);
    if (!no) { printf("  Jogo nao encontrado.\n"); return; }

    printf("\n  [1] Marcar como Zerado\n  [2] Marcar como Platinado\n");
    int op = menu_ler_inteiro("Opcao: ");
    if (op != 1 && op != 2) { printf("  Cancelado.\n"); return; }

    char status_ant[MAX_STATUS];
    strncpy(status_ant, no->dados.status, MAX_STATUS - 1);
    const char *novo_status = (op == 1) ? "Zerado" : "Platinado";

    strncpy(no->dados.status, novo_status, MAX_STATUS - 1);
    historico_push_log(no, status_ant, novo_status);
    historico_push_conquista(no);

    printf("  \"%s\" marcado como %s!\n", no->dados.nome, novo_status);
}

/* ══════════════════════════════════════════════════════════════════
   menu_loop — loop principal do sistema
   ══════════════════════════════════════════════════════════════════ */
void menu_loop(void) {
    int opcao;
    do {
        system(LIMPAR_TELA);
        printf("\n");
        printf("  ╔══════════════════════════════════════════╗\n");
        printf("  ║        BACKLOG MANAGER  v1.0             ║\n");
        printf("  ╠══════════════════════════════════════════╣\n");
        printf("  ║  Jogos no catalogo: %-20d║\n", catalogo_total);
        printf("  ╠══════════════════════════════════════════╣\n");
        printf("  ║  [1]  Cadastrar jogo                     ║\n");
        printf("  ║  [2]  Remover jogo                       ║\n");
        printf("  ║  [3]  Editar jogo                        ║\n");
        printf("  ║  [4]  Buscar jogo                        ║\n");
        printf("  ║  [5]  Listar catalogo                    ║\n");
        printf("  ║  [6]  Ordenar catalogo                   ║\n");
        printf("  ║  [7]  Navegar por carrosseis             ║\n");
        printf("  ║  [8]  Gerenciar backlog (filas)          ║\n");
        printf("  ║  [9]  Historico e pilhas                 ║\n");
        printf("  ║  [10] Conquista (Zerado/Platinado)       ║\n");
        printf("  ║  [11] Salvar dados                       ║\n");
        printf("  ║  [0]  Sair                               ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");
        printf("  Opcao: ");

        if (scanf("%d", &opcao) != 1) {
            menu_limpar_buffer();
            opcao = -1;
        } else {
            menu_limpar_buffer();
        }

        switch (opcao) {
            case  1: _menu_cadastrar();                       break;
            case  2: _menu_remover();                         break;
            case  3: _menu_editar();                          break;
            case  4: _menu_buscar();                          break;
            case  5: catalogo_listar();                       break;
            case  6: _menu_ordenar();                         break;
            case  7: _menu_carrossel();                       break;
            case  8: backlog_menu_interativo();               break;
            case  9: historico_menu_interativo();             break;
            case 10: _menu_conquista();                       break;
            case 11:
                persistencia_salvar(ARQUIVO_DADOS);
                break;
            case  0:
                printf("\n  Salvando dados...\n");
                break;
            default:
                printf("  Opcao invalida.\n");
        }

        if (opcao != 0) {
            printf("\n  Pressione ENTER para continuar...");
            getchar();
        }

    } while (opcao != 0);
}
