#include "catalogo.h"
#include "carrossel.h"
#include "backlog.h"
#include "historico.h"
#include "persistencia.h"
#include "menu.h"

#ifdef _WIN32
  #include <windows.h>
  static void configurar_console_windows(void) {
      SetConsoleOutputCP(65001);
      SetConsoleCP(65001);
      HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
      DWORD  dwMode = 0;
      GetConsoleMode(hOut, &dwMode);
      SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
#endif

static void liberar_tudo(void) {
    historico_liberar_tudo();
    backlog_liberar_tudo();
    carrossel_liberar_tudo();
    NoJogo *cur = catalogo_head;
    while (cur) { NoJogo *p = cur->proximo; free(cur); cur = p; }
    catalogo_head = NULL; catalogo_tail = NULL; catalogo_total = 0;
}

int main(void) {
#ifdef _WIN32
    configurar_console_windows();
#endif
    catalogo_inicializar();
    carrossel_inicializar();
    backlog_inicializar();
    historico_inicializar();
    persistencia_carregar(ARQUIVO_DADOS);
    catalogo_ordenar(ORDEM_ID);   /* garante ordem consistente ao iniciar */
    menu_loop();
    persistencia_salvar(ARQUIVO_DADOS);
    liberar_tudo();
    printf("  Ate a proxima!\n\n");
    return 0;
}
