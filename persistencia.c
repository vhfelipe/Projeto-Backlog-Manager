/* ══════════════════════════════════════════════════════════════════
   persistencia.c  —  Leitura e escrita binária (fread / fwrite)

   REGRA (RN06): apenas DadosJogo é persistido.
   Ponteiros de encadeamento NUNCA são gravados.

   Formato do arquivo:
     [ int32 magic ][ int32 versao ][ int32 total ]
     [ DadosJogo ] [ DadosJogo ] ...
   ══════════════════════════════════════════════════════════════════ */
#include "persistencia.h"
#include "catalogo.h"
#include "carrossel.h"
#include "historico.h"

#define MAGIC   0x424C4D47   /* "BLMG" — Backlog Manager */
#define VERSAO  1

/* ══════════════════════════════════════════════════════════════════
   persistencia_carregar — lê o .bin e reconstrói a lista encadeada
   e os carrosseis. Deve ser chamado PRIMEIRO em main.c.
   ══════════════════════════════════════════════════════════════════ */
int persistencia_carregar(const char *caminho) {
    FILE *f = fopen(caminho, "rb");
    if (!f) {
        /* Arquivo não existe ainda — sistema começa do zero */
        printf("  [INFO] Arquivo de dados nao encontrado. "
               "Iniciando com biblioteca vazia.\n");
        return 0;
    }

    /* Lê cabeçalho */
    int magic = 0, versao = 0, total = 0;
    if (fread(&magic,  sizeof(int), 1, f) != 1 ||
        fread(&versao, sizeof(int), 1, f) != 1 ||
        fread(&total,  sizeof(int), 1, f) != 1 ||
        magic != MAGIC) {
        printf("  [ERRO] Arquivo de dados corrompido ou invalido.\n");
        fclose(f);
        return -1;
    }

    int carregados = 0;
    DadosJogo d;
    for (int i = 0; i < total; i++) {
        if (fread(&d, sizeof(DadosJogo), 1, f) != 1) {
            printf("  [AVISO] Leitura interrompida no registro %d.\n", i);
            break;
        }
        NoJogo *no = catalogo_inserir(d);
        if (no) {
            carrossel_inserir(no);
            /* Reconstroi pilhas de conquista para jogos ja zerados/platinados (RN06) */
            if (strcmp(no->dados.status, "Zerado")    == 0 ||
                strcmp(no->dados.status, "Platinado") == 0) {
                historico_push_conquista(no);
            }
            carregados++;
        }
    }

    fclose(f);
    printf("  [INFO] %d jogo(s) carregado(s) de \"%s\".\n",
           carregados, caminho);
    return carregados;
}

/* ══════════════════════════════════════════════════════════════════
   persistencia_salvar — percorre a lista encadeada e grava apenas
   os campos de DadosJogo (sem ponteiros).
   ══════════════════════════════════════════════════════════════════ */
int persistencia_salvar(const char *caminho) {
    FILE *f = fopen(caminho, "wb");
    if (!f) {
        perror("  [ERRO] Nao foi possivel abrir arquivo para salvar");
        return -1;
    }

    /* Cabeçalho */
    int magic  = MAGIC;
    int versao = VERSAO;
    int total  = catalogo_total;
    fwrite(&magic,  sizeof(int), 1, f);
    fwrite(&versao, sizeof(int), 1, f);
    fwrite(&total,  sizeof(int), 1, f);

    /* Dados — apenas DadosJogo, sem ponteiros */
    int salvos = 0;
    NoJogo *cur = catalogo_head;
    while (cur) {
        if (fwrite(&cur->dados, sizeof(DadosJogo), 1, f) == 1) salvos++;
        cur = cur->proximo;
    }

    fclose(f);
    printf("  [INFO] %d jogo(s) salvo(s) em \"%s\".\n", salvos, caminho);
    return salvos;
}
