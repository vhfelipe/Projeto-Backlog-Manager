/* ══════════════════════════════════════════════════════════════════
   plataformas.c  —  Lista canônica de plataformas + normalização

   Cada entrada da tabela possui:
     .canonical  → nome oficial que será armazenado e exibido
     .aliases[]  → variações aceitas (case-insensitive, sem acento)

   plataforma_normalizar("ps4")      → "PS4"
   plataforma_normalizar("PS4")      → "PS4"
   plataforma_normalizar("playstation 4") → "PS4"
   plataforma_normalizar("Pizza")    → NULL  (rejeitada)
   ══════════════════════════════════════════════════════════════════ */
#include "plataformas.h"

/* ─── Tabela canônica ────────────────────────────────────────────── */
typedef struct {
    const char *canonical;
    const char *aliases[20];   /* último elemento deve ser NULL */
} Plataforma;

static const Plataforma TABELA[] = {

    /* ── PC / Lojas digitais ──────────────────────────────────────── */
    { "PC",
      { "pc", "windows", "win", "computador", "computer", NULL } },

    { "Steam",
      { "steam", "steam pc", NULL } },

    { "Epic Games",
      { "epic games", "epic", "egs", "epicgames", "epic game store",
        "epic games store", NULL } },

    { "GOG",
      { "gog", "good old games", "gog.com", NULL } },

    { "Battle.net",
      { "battle.net", "battlenet", "blizzard", "bnet", NULL } },

    { "EA App",
      { "ea app", "ea", "origin", "ea origin", NULL } },

    { "Ubisoft Connect",
      { "ubisoft connect", "ubisoft", "uplay", "ubi", NULL } },

    /* ── Sony PlayStation ─────────────────────────────────────────── */
    { "PlayStation",
      { "playstation", "ps", "ps1", "psx", "playstation 1",
        "playstation1", NULL } },

    { "PS2",
      { "ps2", "playstation 2", "playstation2", NULL } },

    { "PS3",
      { "ps3", "playstation 3", "playstation3", NULL } },

    { "PS4",
      { "ps4", "playstation 4", "playstation4", "ps 4", NULL } },

    { "PS5",
      { "ps5", "playstation 5", "playstation5", "ps 5", NULL } },

    { "PSP",
      { "psp", "playstation portable", NULL } },

    { "PS Vita",
      { "ps vita", "psvita", "vita", "playstation vita", NULL } },

    /* ── Microsoft Xbox ───────────────────────────────────────────── */
    { "Xbox",
      { "xbox", "xbox original", "xbox 1", "xbox1", NULL } },

    { "Xbox 360",
      { "xbox 360", "xbox360", "x360", "360", NULL } },

    { "Xbox One",
      { "xbox one", "xboxone", "xone", "xb1", NULL } },

    { "Xbox Series X",
      { "xbox series x", "xsx", "series x", "xbox series", NULL } },

    { "Xbox Series S",
      { "xbox series s", "xss", "series s", NULL } },

    /* ── Nintendo consoles de mesa ────────────────────────────────── */
    { "NES",
      { "nes", "nintendo entertainment system", "famicom", NULL } },

    { "SNES",
      { "snes", "super nintendo", "super nes", "super famicom",
        "sfc", NULL } },

    { "Nintendo 64",
      { "nintendo 64", "n64", "nintendo64", "n 64", NULL } },

    { "GameCube",
      { "gamecube", "gcn", "ngc", "game cube", "nintendo gamecube",
        NULL } },

    { "Wii",
      { "wii", "nintendo wii", NULL } },

    { "Wii U",
      { "wii u", "wiiu", "nintendo wii u", NULL } },

    { "Nintendo Switch",
      { "nintendo switch", "switch", "ns", "oled switch",
        "switch oled", "switch lite", NULL } },

    /* ── Nintendo portáteis ───────────────────────────────────────── */
    { "Game Boy",
      { "game boy", "gameboy", "gb", "game boy color", "gbc",
        "gameboy color", NULL } },

    { "Game Boy Advance",
      { "game boy advance", "gba", "gameboy advance",
        "game boy advance sp", "gba sp", NULL } },

    { "Nintendo DS",
      { "nintendo ds", "nds", "ds", "ds lite", "dsi", NULL } },

    { "Nintendo 3DS",
      { "nintendo 3ds", "3ds", "new 3ds", "3ds xl", "2ds", NULL } },

    /* ── Sega ─────────────────────────────────────────────────────── */
    { "Sega Genesis",
      { "sega genesis", "genesis", "mega drive", "megadrive",
        "sega mega drive", "smd", NULL } },

    { "Sega Saturn",
      { "sega saturn", "saturn", NULL } },

    { "Sega Dreamcast",
      { "sega dreamcast", "dreamcast", "dc", NULL } },

    { "Game Gear",
      { "game gear", "gamegear", "sega game gear", NULL } },

    /* ── Mobile ───────────────────────────────────────────────────── */
    { "Android",
      { "android", "google play", "android mobile", NULL } },

    { "iOS",
      { "ios", "iphone", "ipad", "apple mobile", "app store",
        NULL } },

    /* ── Outros ───────────────────────────────────────────────────── */
    { "Mac",
      { "mac", "macos", "osx", "os x", "apple mac", NULL } },

    { "Atari",
      { "atari", "atari 2600", "atari 2600", "atari vcs",
        "atari 5200", "atari 7800", NULL } },
};

#define N_PLATAFORMAS ((int)(sizeof(TABELA) / sizeof(TABELA[0])))

/* ─── Utilidade: copia em minúsculas + strip de espaços ─────────── */
static void _para_lower_trim(const char *src, char *dst, int max) {
    /* Avança espaços iniciais */
    while (*src == ' ') src++;

    int i = 0;
    while (*src && i < max - 1) {
        dst[i++] = (char)tolower((unsigned char)*src++);
    }
    dst[i] = '\0';

    /* Remove espaços finais */
    while (i > 0 && dst[i - 1] == ' ') dst[--i] = '\0';
}

/* ══════════════════════════════════════════════════════════════════
   plataforma_normalizar — compara entrada (case-insensitive, trimmed)
   contra o nome canônico e todos os aliases de cada plataforma.
   Retorna o nome canônico se encontrado, NULL caso contrário.
   ══════════════════════════════════════════════════════════════════ */
const char *plataforma_normalizar(const char *entrada) {
    if (!entrada || entrada[0] == '\0') return NULL;

    char low[MAX_PLATAFORMA];
    _para_lower_trim(entrada, low, MAX_PLATAFORMA);
    if (low[0] == '\0') return NULL;

    for (int p = 0; p < N_PLATAFORMAS; p++) {
        /* Compara com o nome canônico em minúsculas */
        char canon_low[MAX_PLATAFORMA];
        _para_lower_trim(TABELA[p].canonical, canon_low, MAX_PLATAFORMA);
        if (strcmp(low, canon_low) == 0) return TABELA[p].canonical;

        /* Compara com cada alias (já estão em minúsculas na tabela) */
        for (int a = 0; TABELA[p].aliases[a] != NULL; a++) {
            if (strcmp(low, TABELA[p].aliases[a]) == 0)
                return TABELA[p].canonical;
        }
    }
    return NULL;
}

/* ══════════════════════════════════════════════════════════════════
   plataforma_listar_validas — exibe a tabela em 3 colunas
   ══════════════════════════════════════════════════════════════════ */
void plataforma_listar_validas(void) {
    printf("\n  Plataformas aceitas:\n");
    printf("  %-22s %-22s %-22s\n",
           "──────────────────", "──────────────────", "──────────────────");
    for (int i = 0; i < N_PLATAFORMAS; i += 3) {
        printf("  %-22s %-22s %-22s\n",
               TABELA[i].canonical,
               (i+1 < N_PLATAFORMAS) ? TABELA[i+1].canonical : "",
               (i+2 < N_PLATAFORMAS) ? TABELA[i+2].canonical : "");
    }
    printf("\n  Variações aceitas: \"ps4\" = \"PS4\", \"switch\" = \"Nintendo Switch\", etc.\n");
}
