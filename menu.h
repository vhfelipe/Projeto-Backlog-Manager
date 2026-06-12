/* ══════════════════════════════════════════════════════════════════
   menu.h  —  Interface CLI: captura, valida e roteia chamadas
   ══════════════════════════════════════════════════════════════════ */
#ifndef MENU_H
#define MENU_H

#include "tipos.h"

/* Executa o loop principal do menu. Retorna quando o usuário sair. */
void menu_loop(void);

/* Utilitários de entrada */
void   menu_limpar_buffer(void);
float  menu_ler_nota(void);                    /* valida [0.0, 10.0] (RN08) */
int    menu_ler_inteiro(const char *prompt);
void   menu_ler_string(const char *prompt,
                       char *destino, int tamanho);

#endif /* MENU_H */
