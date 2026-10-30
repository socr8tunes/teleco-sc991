/*
 * UI case runner: each line of a case file is   [{expression text}] key script => expected result
 * A fresh calculator is used for every line. Lines starting with # are comments.
 */
#include <stdio.h>
#include <string.h>
#include "sim_keys.h"

static app_t A;
static int fails, checks;

static void trim(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ')) s[--n] = 0;
}

int main(int argc, char **argv)
{
    FILE *f;
    char line[512];
    int ln = 0;
    if (argc < 2) { fprintf(stderr, "usage: test_ui cases.txt\n"); return 2; }
    f = fopen(argv[1], "r");
    if (!f) { fprintf(stderr, "cannot open %s\n", argv[1]); return 2; }
    while (fgets(line, sizeof line, f)) {
        char *sep, *keys = line, got[160], exp[160];
        const char *bad = 0;
        ln++;
        trim(line);
        if (!line[0] || line[0] == '#') continue;
        sep = strstr(line, "=>");
        if (!sep) { printf("%s:%d: no =>\n", argv[1], ln); fails++; continue; }
        *sep = 0;
        strcpy(exp, sep + 2);
        { char *e = exp; while (*e == ' ') e++; memmove(exp, e, strlen(e) + 1); }
        trim(keys);
        app_init(&A);
        app_tick(&A, 5000);
        app_key(&A, K_AC);
        if (keys[0] == '{') {
            char *end = strchr(keys, '}');
            if (!end) { printf("%s:%d: missing }\n", argv[1], ln); fails++; continue; }
            *end = 0;
            if (ui_set_expr_text(&A, keys + 1)) { printf("%s:%d: cannot load expression\n", argv[1], ln); fails++; continue; }
            keys = end + 1;
        }
        checks++;
        if (sim_run_keys(&A, keys, &bad) < 0) {
            printf("%s:%d: unknown key near '%s'\n", argv[1], ln, bad);
            fails++;
            continue;
        }
        sim_result_text(&A, got, sizeof got);
        if (strcmp(exp, "@expr") == 0) continue;
        if (strncmp(exp, "expr:", 5) == 0) {
            sim_expr_text(&A, got, sizeof got);
            if (strcmp(got, exp + 5) != 0) { printf("%s:%d: [%s] expr got '%s' expected '%s'\n", argv[1], ln, keys, got, exp + 5); fails++; }
            continue;
        }
        if (strcmp(got, exp) != 0) {
            printf("%s:%d: [%s] got '%s' expected '%s'\n", argv[1], ln, keys, got, exp);
            fails++;
        }
    }
    fclose(f);
    printf("%s: %d cases, %d failures\n", argv[1], checks, fails);
    return fails != 0;
}
