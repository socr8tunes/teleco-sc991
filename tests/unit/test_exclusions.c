/*
 * SC991 Exam exclusion guard and menu-path test.
 *  1. The set of token labels the calculator can produce must equal tests/exam/approved_tokens.txt. Adding a token
 *     (a new function, a statement separator, a program/table/graph token ...) fails this test until the list is
 *     reviewed and edited on purpose.
 *  2. Every item of every menu is visited by key presses (open, DOWN x i, EXE). Each must leave the app on a valid
 *     screen, render a non-empty label, insert only approved tokens, and be backed out with AC.
 *  3. No menu label may name a forbidden capability (programming, macros, spreadsheet, graphing, TABLE/CALC while
 *     on hold, wireless/import/export, AI).
 *  usage: test_exclusions approved_tokens.txt
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_keys.h"
#include "parser/tokinfo.h"

static app_t A;
static int fails, items;
static char approved[512][24];
static int n_approved;

/* printable form of a label: raised-run markers \x01 \x02 become { }, other control bytes become ? */
static const char *norm(const char *s)
{
    static char b[8][64];
    static int k;
    char *o = b[k++ & 7];
    size_t i, n = 0;
    for (i = 0; s[i] && n + 1 < 64; i++) o[n++] = s[i] == 1 ? '{' : s[i] == 2 ? '}' : ((unsigned char)s[i] < 32 || (unsigned char)s[i] > 126) ? '?' : s[i];
    o[n] = 0;
    return o;
}

static int is_approved(const char *s)
{
    int i;
    s = norm(s);
    for (i = 0; i < n_approved; i++) if (!strcmp(approved[i], s)) return 1;
    return 0;
}

static const char *const FORBIDDEN[] = {"prog", "macro", "script", "sheet", "graph", "plot", "table", "calc ", "wireless", "bluetooth",
                                        "wifi", "internet", "import", "export", "usb", "ai ", "symbolic", "formula", "loop"};

static void lower(char *d, const char *s, size_t cap) { size_t i; for (i = 0; i + 1 < cap && s[i]; i++) d[i] = (char)tolower((unsigned char)s[i]); d[i] = 0; }

static void fresh(void) { app_init(&A); app_tick(&A, 5000); app_key(&A, K_AC); }

int main(int argc, char **argv)
{
    FILE *f;
    char line[1024], lab[64];
    int t, id, i, k;
    static char seen[512][24];
    int n_seen = 0;

    if (argc < 2 || !(f = fopen(argv[1], "r"))) { fprintf(stderr, "usage: test_exclusions approved_tokens.txt\n"); return 2; }
    while (fgets(line, sizeof line, f) && n_approved < 512) {
        size_t n = strlen(line);
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = 0;
        if (!n || line[0] == '#') continue;
        if (n >= sizeof approved[0]) { printf("approved label too long: %s\n", line); fails++; continue; }
        memcpy(approved[n_approved], line, n + 1); n_approved++;
    }
    fclose(f);

    /* 1. token label set == approved list */
    for (t = 1; t < T_LAST; t++) {
        tok_t tk; const char *s; int dup = 0;
        tk.t = (uint8_t)t; tk.a = 0;
        s = norm(tok_label(tk, lab, sizeof lab));
        if (!s || !s[0]) { printf("token %d has no label\n", t); fails++; continue; }
        for (i = 0; i < n_seen; i++) if (!strcmp(seen[i], s)) dup = 1;
        if (!dup && n_seen < 512) { { size_t sl = strlen(s); if (sl >= sizeof seen[0]) sl = sizeof seen[0] - 1; memcpy(seen[n_seen], s, sl); seen[n_seen][sl] = 0; } n_seen++; }
        if (!is_approved(s)) { printf("UNAPPROVED token %d label '%s' (add to tests/exam/approved_tokens.txt after review)\n", t, s); fails++; }
    }
    for (i = 0; i < n_approved; i++) {
        int found = 0;
        for (k = 0; k < n_seen; k++) if (!strcmp(seen[k], approved[i])) found = 1;
        if (!found) { printf("approved label '%s' no longer exists (remove it from the list)\n", approved[i]); fails++; }
    }

    if (argc > 2 && !strcmp(argv[2], "--paths")) {   /* tab separated: key path, inserted token label (for tests/tools/make_menu_paths.py) */
        int j, nm = ui_menu_item_count(&A, MN_MAIN);
        for (j = 0; j < nm; j++) {
            int sub, n2;
            fresh(); ui_menu_open(&A, MN_MAIN);
            for (i = 0; i < j; i++) app_key(&A, K_DOWN);
            app_key(&A, K_EXE);
            if (A.screen != SCR_MENU) continue;
            sub = A.menu_id[A.menu_sp - 1];
            if (sub == MN_MAIN || sub == MN_CONST || sub == MN_CONV || sub == MN_STATVARS) continue;
            n2 = ui_menu_item_count(&A, sub);
            for (i = 0; i < n2; i++) {
                int before;
                fresh(); ui_menu_open(&A, MN_MAIN);
                for (k = 0; k < j; k++) app_key(&A, K_DOWN);
                app_key(&A, K_EXE);
                for (k = 0; k < i; k++) app_key(&A, K_DOWN);
                before = A.ed.n;
                app_key(&A, K_EXE);
                if (A.screen == SCR_CALC && A.ed.n == before + 1) {
                    tok_t tk = A.ed.t[before]; const char *lb; tk.a = 0;
                    lb = norm(tok_label(tk, lab, sizeof lab));
                    printf("MENU%.*s EXE%.*s EXE\t%s\n", 0, "", 0, "", lb);   /* placeholder, replaced below */
                    printf("PATH %d %d %s\n", j, i, lb);
                }
            }
        }
        return 0;
    }
    /* 2 + 3. menu paths */
    for (id = 0; id < MN_COUNT; id++) {
        int n;
        fresh();
        n = ui_menu_item_count(&A, id);
        if (n <= 0 && id != MN_MODE) { printf("menu %d has no items\n", id); fails++; continue; }
        for (i = 0; i < n; i++) {
            char label[96], low[96];
            int j, before, sp = 0;
            fresh();
            ui_menu_item_label(&A, id, i, label, sizeof label);
            if (!label[0]) { printf("menu %d item %d: empty label\n", id, i); fails++; }
            lower(low, label, sizeof low);
            for (j = 0; j < (int)(sizeof FORBIDDEN / sizeof FORBIDDEN[0]); j++)
                if (strstr(low, FORBIDDEN[j])) { printf("menu %d item %d '%s': forbidden capability '%s'\n", id, i, label, FORBIDDEN[j]); fails++; }
            ui_menu_open(&A, id);
            if (A.screen != SCR_MENU) { printf("menu %d did not open\n", id); fails++; continue; }
            for (j = 0; j < i; j++) app_key(&A, K_DOWN);
            before = A.ed.n;
            app_key(&A, K_EXE);
            items++;
            if ((int)A.screen > SCR_GRID) { printf("menu %d item %d: invalid screen %d\n", id, i, A.screen); fails++; }
            for (j = before; j < A.ed.n; j++) {
                tok_t base = A.ed.t[j]; const char *s; base.a = 0;   /* labels of indexed tokens (variable, constant ...) are checked by type */
                s = tok_label(base, lab, sizeof lab);
                if (!is_approved(s)) { printf("menu %d item %d '%s' inserted unapproved token '%s'\n", id, i, label, s); fails++; }
            }
            while (A.screen != SCR_CALC && sp++ < 8) app_key(&A, K_AC);
            if (A.screen != SCR_CALC) { printf("menu %d item %d: cannot return to the calculation screen\n", id, i); fails++; }
        }
    }
    if (argc > 2 && !strcmp(argv[2], "--dump")) {
        for (i = 0; i < n_seen; i++) for (k = i + 1; k < n_seen; k++) if (strcmp(seen[i], seen[k]) > 0) { char tmp[24]; strcpy(tmp, seen[i]); strcpy(seen[i], seen[k]); strcpy(seen[k], tmp); }
        for (i = 0; i < n_seen; i++) puts(seen[i]);
        return 0;
    }
    printf("test_exclusions: %d tokens, %d menu items visited, %d failed\n", n_seen, items, fails);
    return fails ? 1 : 0;
}
