/*
 * Differential test runner.
 *
 * Reads vector files (or "-" for stdin) with lines of the form
 *     <op> <a> <b> <expected-result> <expected-status>
 * where op is add|sub|mul|div|sqrt|parse. For sqrt, b is "-". For parse, a is
 * the raw input string and b is "-". Lines starting with '#' are ignored.
 *
 * The expected values come from tests/differential/run_diff.py, which uses
 * Python's decimal module as the reference implementation.
 */
#include <stdio.h>
#include <string.h>

#include "numeric/decimal.h"

enum { N_OPS = 6, MAX_PRINTED_FAILURES = 25 };

static const char *const k_ops[N_OPS] = { "add", "sub", "mul", "div", "sqrt", "parse" };

static unsigned long g_total[N_OPS];
static unsigned long g_failed[N_OPS];
static unsigned long g_printed;

static void report(const char *file, unsigned long lineno, const char *line,
                   const char *why, const char *got, const char *got_status)
{
    if (g_printed++ < MAX_PRINTED_FAILURES) {
        printf("FAIL %s:%lu (%s)\n  vector: %s\n  got:    %s %s\n", file, lineno, why, line, got, got_status);
    }
}

static int op_index(const char *op)
{
    int i;
    for (i = 0; i < N_OPS; i++) {
        if (strcmp(op, k_ops[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static int run_stream(FILE *f, const char *name)
{
    char line[512];
    unsigned long lineno = 0;

    while (fgets(line, sizeof line, f)) {
        char op[16], sa[128], sb[128], want[64], want_st[16];
        char got[DEC_STR_MAX];
        char shown[512];
        dec_t a, b, r;
        dec_status st;
        int oi;
        size_t len;

        lineno++;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        strcpy(shown, line);
        len = strlen(shown);
        if (len > 0 && shown[len - 1] == '\n') {
            shown[len - 1] = '\0';
        }

        if (sscanf(line, "%15s %127s %127s %63s %15s", op, sa, sb, want, want_st) != 5 ||
            (oi = op_index(op)) < 0) {
            printf("BAD LINE %s:%lu: %s\n", name, lineno, shown);
            g_printed++;
            return 1;
        }
        g_total[oi]++;

        if (oi == 5) {
            st = dec_from_str(&r, sa);
        } else {
            if (dec_from_str(&a, sa) != DEC_OK ||
                (oi != 4 && dec_from_str(&b, sb) != DEC_OK)) {
                g_failed[oi]++;
                report(name, lineno, shown, "operand failed to parse", "-", "-");
                continue;
            }
            switch (oi) {
            case 0: st = dec_add(&r, &a, &b); break;
            case 1: st = dec_sub(&r, &a, &b); break;
            case 2: st = dec_mul(&r, &a, &b); break;
            case 3: st = dec_div(&r, &a, &b); break;
            default: st = dec_sqrt(&r, &a); break;
            }
        }

        dec_to_str(&r, got, sizeof got);
        if (!dec_is_valid(&r)) {
            g_failed[oi]++;
            report(name, lineno, shown, "non-canonical result", got, dec_status_name(st));
        } else if (strcmp(got, want) != 0 || strcmp(dec_status_name(st), want_st) != 0) {
            g_failed[oi]++;
            report(name, lineno, shown, "mismatch", got, dec_status_name(st));
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    unsigned long total = 0, failed = 0;
    int i;
    int bad = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <vectors-file | -> ...\n", argv[0]);
        return 2;
    }

    for (i = 1; i < argc && !bad; i++) {
        if (strcmp(argv[i], "-") == 0) {
            bad = run_stream(stdin, "<stdin>");
        } else {
            FILE *f = fopen(argv[i], "r");
            if (!f) {
                fprintf(stderr, "cannot open %s\n", argv[i]);
                return 2;
            }
            bad = run_stream(f, argv[i]);
            fclose(f);
        }
    }

    for (i = 0; i < N_OPS; i++) {
        printf("%-6s %9lu vectors, %lu failed\n", k_ops[i], g_total[i], g_failed[i]);
        total += g_total[i];
        failed += g_failed[i];
    }
    printf("total  %9lu vectors, %lu failed\n", total, failed);
    return (failed || bad) ? 1 : 0;
}
