/* ══════════════════════════════════════════════════════════════════
   persistencia.h  —  Leitura e escrita binária do catálogo
   ══════════════════════════════════════════════════════════════════ */
#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H

#include "tipos.h"

/* Retorna a quantidade de registros carregados, ou -1 em erro */
int persistencia_carregar(const char *caminho);

/* Retorna a quantidade de registros salvos, ou -1 em erro */
int persistencia_salvar(const char *caminho);

#endif /* PERSISTENCIA_H */
