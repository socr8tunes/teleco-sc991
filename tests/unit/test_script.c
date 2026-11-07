/*
 * Runs a case file through the calculator.
 *   # comment
 *   :deg                     settings are applied (see teleco_cli)
 *   expression => expected   compared as text (ASCII output)
 *   expression ~> expected   numeric, relative tolerance 1E-8 (numerical-method results: integrals, derivatives, Solve);
 *                            non-numeric expected values (errors) are compared as text
 * Every case runs after ":reset" + the settings given since the last "@@" line.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "calc_script.h"

static int near_equal(const char *got, const char *want)
{
    char *e1, *e2, first[128];
    const char *bar = strstr(got, " | ");
    double a, b;
    if (bar && !strstr(want, " | ") && (size_t)(bar - got) < sizeof first) { memcpy(first, got, (size_t)(bar - got)); first[bar - got] = 0; got = first; }   /* Solve: compare the root, not the residual line */
    a = strtod(got, &e1); b = strtod(want, &e2);
    if (e1 == got || *e1 || e2 == want || *e2) return strcmp(got, want) == 0;
    return fabs(a - b) <= 1e-8 * (fabs(a) > fabs(b) ? fabs(a) : fabs(b));
}

int main(int argc, char **argv)
{
    static script_t sc;
    FILE *f;
    char line[1024], out[512];
    int total = 0, fail = 0, lineno = 0;

    if (argc < 2) { fprintf(stderr, "usage: %s casefile\n", argv[0]); return 2; }
    f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 2; }
    script_init(&sc);
    while (fgets(line, sizeof line, f)) {
        char *arrow, *expect, *expr;
        int tolerant = 0;
        size_t n = strlen(line);
        lineno++;
        while (n && (line[n - 1] == '\n' || line[n - 1] == '\r')) line[--n] = 0;
        if (!n || line[0] == '#') continue;
        if (line[0] == ':') { script_line(&sc, line, out, sizeof out); continue; }
        arrow = strstr(line, " => ");
        if (!arrow && (arrow = strstr(line, " ~> ")) != NULL) tolerant = 1;
        if (!arrow) { printf("%s:%d: malformed case\n", argv[1], lineno); fail++; continue; }
        *arrow = 0;
        expr = line;
        expect = arrow + 4;
        total++;
        if (!script_line(&sc, expr, out, sizeof out)) strcpy(out, "(no output)");
        else {
            char aux[256], *nl;
            if (sc.ctx.nres > 1 && script_aux(&sc, aux, sizeof aux)) {
                size_t l = strlen(out);
                while ((nl = strchr(aux, '\n')) != NULL) { memmove(nl + 3, nl + 1, strlen(nl)); memcpy(nl, " | ", 3); }
                snprintf(out + l, sizeof out - l, " | %s", aux);
            }
        }
        if (tolerant ? !near_equal(out, expect) : strcmp(out, expect) != 0) {
            fail++;
            printf("FAIL %s:%d: %s\n   expected: %s\n   got:      %s\n", argv[1], lineno, expr, expect, out);
        }
    }
    fclose(f);
    printf("%s: %d cases, %d failed\n", argv[1], total, fail);
    return fail ? 1 : 0;
}
