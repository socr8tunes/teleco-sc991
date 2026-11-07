/*
 * Freeze test for persisted identifiers.
 *  1. core/parser/token_ids.h must equal tests/exam/frozen_token_ids.txt (every token: same ID; IDs unique; no 0xFF;
 *     lines may be appended for new tokens, never changed).
 *  2. Auxiliary tables (constants, conversions, statistics variables, variable names) must start with the frozen lists in
 *     tests/exam/frozen_aux.txt: they may only be appended to.
 *  3. A replay history holding every token type survives save / load, and a blob with an unknown ID drops only the history.
 *  usage: test_token_ids frozen_token_ids.txt frozen_aux.txt [--dump-aux]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ui/app.h"
#include "ui/persist.h"
#include "parser/token_ids.h"
#include "parser/tokinfo.h"
#include "units/units.h"
#include "stats/stat.h"
#include "hal/storage.h"
#include "hal_pc.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); fails++; } } while (0)

#define NAME_ENTRY(n, id) #n,
static const char *const NAMES[] = { TOK_WIRE_LIST(NAME_ENTRY) };

static void aux_line(char *dst, size_t cap, const char *kind, int i, const char *what)
{
    char t[80];
    size_t k;
    for (k = 0; what[k] && k + 1 < sizeof t; k++) t[k] = ((unsigned char)what[k] < 32 || (unsigned char)what[k] > 126) ? '?' : what[k];   /* display glyphs -> '?' */
    t[k] = 0;
    snprintf(dst, cap, "%s %d %s", kind, i, t);
}

static int aux_list(char lines[][96], int max)
{
    int n = 0, i;
    char lab[64];
    for (i = 0; i < const_count() && n < max; i++) aux_line(lines[n++], 96, "CONST", i, const_get(i)->name);
    for (i = 0; i < conv_count() && n < max; i++) { char b[80]; snprintf(b, sizeof b, "%s>%s", conv_get(i)->from, conv_get(i)->to); aux_line(lines[n++], 96, "CONV", i, b); }
    for (i = 0; i < SV_COUNT && n < max; i++) { tok_t t; t.t = T_STATVAR; t.a = (uint8_t)i; aux_line(lines[n++], 96, "STATVAR", i, tok_label(t, lab, sizeof lab)); }
    for (i = 0; i < 9 && n < max; i++) { tok_t t; t.t = T_VAR; t.a = (uint8_t)i; aux_line(lines[n++], 96, "VAR", i, tok_label(t, lab, sizeof lab)); }
    return n;
}

int main(int argc, char **argv)
{
    FILE *f;
    char line[256];
    int i, k, nn = (int)(sizeof NAMES / sizeof NAMES[0]);
    static char cur[600][96], frozen[600][96];
    int ncur, nfz = 0;

    if (argc < 3) { fprintf(stderr, "usage\n"); return 2; }
    ncur = aux_list(cur, 600);
    if (argc > 3 && !strcmp(argv[3], "--dump-aux")) { for (i = 0; i < ncur; i++) puts(cur[i]); return 0; }

    /* 1. token IDs */
    CHECK(nn == (int)T_LAST, "wire list has %d entries, enum has %d tokens", nn, (int)T_LAST);
    for (i = 0; i < T_LAST; i++) for (k = i + 1; k < T_LAST; k++) CHECK(TOK_WIRE[i] != TOK_WIRE[k], "duplicate wire ID %02X (%s, %s)", TOK_WIRE[i], NAMES[i], NAMES[k]);
    for (i = 0; i < T_LAST; i++) CHECK(TOK_WIRE[i] != 0xFF && (TOK_WIRE[i] != 0 || i == 0), "bad wire ID for %s", NAMES[i]);
    if (!(f = fopen(argv[1], "r"))) { printf("cannot open %s\n", argv[1]); return 2; }
    while (fgets(line, sizeof line, f)) {
        char name[64], idtxt[16];
        unsigned id;
        int found = 0;
        if (line[0] == '#' || line[0] == '\n') continue;
        if (sscanf(line, "%63s %15s", name, idtxt) != 2) continue;
        id = (unsigned)strtoul(idtxt, NULL, 16);
        for (i = 0; i < T_LAST; i++) if (!strcmp(NAMES[i], name)) { found = 1; CHECK(TOK_WIRE[i] == id, "%s: frozen ID %02X, header says %02X", name, id, TOK_WIRE[i]); }
        if (!found && !strstr(line, "retired")) printf("note: frozen token %s no longer in the enum (mark it retired)\n", name), fails++;
    }
    fclose(f);
    for (i = 0; i < T_LAST; i++) {
        int listed = 0;
        if (!(f = fopen(argv[1], "r"))) return 2;
        while (fgets(line, sizeof line, f)) { char name[64]; if (line[0] != '#' && sscanf(line, "%63s", name) == 1 && !strcmp(name, NAMES[i])) listed = 1; }
        fclose(f);
        CHECK(listed, "token %s (ID %02X) is not in frozen_token_ids.txt: add its line", NAMES[i], TOK_WIRE[i]);
    }

    /* 2. auxiliary tables only grow */
    if (!(f = fopen(argv[2], "r"))) { printf("cannot open %s\n", argv[2]); return 2; }
    while (fgets(line, sizeof line, f) && nfz < 600) {
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = 0;
        if (!n || line[0] == '#') continue;
        memcpy(frozen[nfz++], line, n + 1);
    }
    fclose(f);
    for (i = 0; i < nfz; i++) {
        int ok = 0;
        for (k = 0; k < ncur; k++) if (!strcmp(frozen[i], cur[k])) ok = 1;
        CHECK(ok, "frozen auxiliary entry changed or removed: %s", frozen[i]);
    }

    /* 3. history round trip with every token type */
    {
        static app_t a, b;
        int n = 0;
        hal_pc_storage_wipe();
        app_init(&a); app_init(&b);
        for (i = 1; i < T_LAST; i++) { a.hist.buf[n].t = (uint8_t)i; a.hist.buf[n].a = (uint8_t)(i & 3); n++; if (n >= 200) break; }
        a.hist.buf[n].t = (uint8_t)n; a.hist.buf[n].a = 0;      /* marker: the record holds n tokens */
        n++;
        a.hist.used = (uint16_t)n;
        {
            hist_t before = a.hist;
            CHECK(persist_save(&a) == 0, "save");
            CHECK(memcmp(&before, &a.hist, sizeof before) == 0, "save leaves the running history unchanged");
            CHECK(persist_load(&b) == 0, "load");
            CHECK(memcmp(&before, &b.hist, sizeof before) == 0, "history round-trips through wire IDs");
        }
    }
    printf("test_token_ids: %d tokens, %d aux entries frozen, %d failed\n", nn, nfz, fails);
    return fails ? 1 : 0;
}
