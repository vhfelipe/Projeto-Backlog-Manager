# ══════════════════════════════════════════════════════════════════
# Makefile — Backlog Manager
# Compilação: make        Limpeza: make clean
# ══════════════════════════════════════════════════════════════════

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic -g
TARGET  = backlog_manager
SRCS    = main.c catalogo.c carrossel.c backlog.c historico.c \
          persistencia.c menu.c plataformas.c
OBJS    = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build OK: ./$(TARGET)"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

distclean: clean
	rm -f biblioteca.bin

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes ./$(TARGET)

.PHONY: all clean distclean valgrind
