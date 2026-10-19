/*
 * Differential runner for the function library.
 *
 * Line format:  <fn> <mode> <a> <b> <expected> <status>
 *   mode: D | R | G (angle unit) or "-"
 *   b:    "-" for unary functions
 * Expected values come from tests/differential/run_fn_diff.py (mpmath reference).
 */
#include <stdio.h>
#include <string.h>

#include "functions/mathfn.h"

enum { MAX_PRINTED = 25 };

typedef dec_status (*unary_fn)(dec_t *, const dec_t *);
typedef dec_status (*binary_fn)(dec_t *, const dec_t *, const dec_t *);
typedef dec_status (*trig_fn)(dec_t *, const dec_t *, ang_t);
typedef dec_status (*trig2_fn)(dec_t *, const dec_t *, const dec_t *, ang_t);

union fnptr {
    unary_fn u;
    binary_fn b;
    trig_fn t;
    trig2_fn t2;
};

struct entry {
    const char *name;
    int kind; /* 1 unary, 2 binary, 3 trig, 4 trig2 */
    union fnptr fn;
};

static const struct entry k_table[] = {
    { "exp", 1, { .u = fn_exp } },   { "exp10", 1, { .u = fn_exp10 } },
    { "ln", 1, { .u = fn_ln } },    { "log10", 1, { .u = fn_log10 } },
    { "cbrt", 1, { .u = fn_cbrt } },  { "sinh", 1, { .u = fn_sinh } },
    { "cosh", 1, { .u = fn_cosh } },  { "tanh", 1, { .u = fn_tanh } },
    { "asinh", 1, { .u = fn_asinh } }, { "acosh", 1, { .u = fn_acosh } },
    { "atanh", 1, { .u = fn_atanh } }, { "fact", 1, { .u = fn_fact } },
    { "floor", 1, { .u = fn_floor } }, { "trunc", 1, { .u = fn_trunc } },
    { "ceil", 1, { .u = fn_ceil } },  { "frac", 1, { .u = fn_frac } },
    { "log", 2, { .b = fn_log } },   { "pow", 2, { .b = fn_pow } },
    { "root", 2, { .b = fn_xroot } }, { "npr", 2, { .b = fn_npr } },
    { "ncr", 2, { .b = fn_ncr } },
    { "sin", 3, { .t = fn_sin } },   { "cos", 3, { .t = fn_cos } },
    { "tan", 3, { .t = fn_tan } },   { "asin", 3, { .t = fn_asin } },
    { "acos", 3, { .t = fn_acos } },  { "atan", 3, { .t = fn_atan } },
    { "atan2", 4, { .t2 = fn_atan2 } },
};

#define N_FN ((int)(sizeof k_table / sizeof k_table[0]))

static unsigned long g_total[N_FN];
static unsigned long g_failed[N_FN];
static unsigned long g_printed;

int main(int argc, char **argv)
{
    char line[512];
    unsigned long lineno = 0;
    FILE *f = stdin;
    unsigned long total = 0, failed = 0;
    int i;

    if (argc > 1 && strcmp(argv[1], "-") != 0) {
        f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "cannot open %s\n", argv[1]);
            return 2;
        }
    }

    while (fgets(line, sizeof line, f)) {
        char fn[16], mode[4], sa[64], sb[64], want[64], wst[16], got[DEC_STR_MAX];
        dec_t a, b, r;
        dec_status st = DEC_OK;
        const struct entry *e = NULL;
        ang_t ang = ANG_RAD;
        int fi;

        lineno++;
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        if (sscanf(line, "%15s %3s %63s %63s %63s %15s", fn, mode, sa, sb, want, wst) != 6) {
            printf("BAD LINE %lu: %s", lineno, line);
            return 2;
        }
        for (fi = 0; fi < N_FN; fi++) {
            if (strcmp(fn, k_table[fi].name) == 0) {
                e = &k_table[fi];
                break;
            }
        }
        if (!e) {
            printf("UNKNOWN FN %lu: %s\n", lineno, fn);
            return 2;
        }
        g_total[fi]++;
        ang = mode[0] == 'D' ? ANG_DEG : (mode[0] == 'G' ? ANG_GRAD : ANG_RAD);

        if (dec_from_str(&a, sa) != DEC_OK || (strcmp(sb, "-") != 0 && dec_from_str(&b, sb) != DEC_OK)) {
            g_failed[fi]++;
            if (g_printed++ < MAX_PRINTED) {
                printf("FAIL line %lu: operand parse: %s", lineno, line);
            }
            continue;
        }
        switch (e->kind) {
        case 1: st = e->fn.u(&r, &a); break;
        case 2: st = e->fn.b(&r, &a, &b); break;
        case 3: st = e->fn.t(&r, &a, ang); break;
        default: st = e->fn.t2(&r, &a, &b, ang); break;
        }
        dec_to_str(&r, got, sizeof got);
        if (!dec_is_valid(&r) || strcmp(got, want) != 0 || strcmp(dec_status_name(st), wst) != 0) {
            g_failed[fi]++;
            if (g_printed++ < MAX_PRINTED) {
                printf("FAIL line %lu: %s", lineno, line);
                printf("   got: %s %s\n", got, dec_status_name(st));
            }
        }
    }

    for (i = 0; i < N_FN; i++) {
        if (g_total[i]) {
            printf("%-6s %8lu vectors, %lu failed\n", k_table[i].name, g_total[i], g_failed[i]);
        }
        total += g_total[i];
        failed += g_failed[i];
    }
    printf("total  %8lu vectors, %lu failed\n", total, failed);
    return failed ? 1 : 0;
}
