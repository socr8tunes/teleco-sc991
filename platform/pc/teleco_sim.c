/*
 * Teleco SC991 simulator (terminal).
 *   teleco_sim [--ascii] [--pbm FILE] [--text] "key script"
 * Without arguments it reads key scripts line by line from stdin and prints
 * the display after each line.
 */
#include <stdio.h>
#include <string.h>
#include "sim_keys.h"

static app_t A;
static lcd_t L;

static void show(int pbm_only, const char *pbm, int text)
{
    char buf[128];
    app_render(&A, &L);
    if (pbm) { FILE *f = fopen(pbm, "w"); if (f) { sim_frame_pbm(&L, f); fclose(f); } }
    if (!pbm_only) sim_frame_ascii(&L, stdout);
    if (text) {
        sim_expr_text(&A, buf, sizeof buf);
        printf("expr: %s\n", buf);
        sim_result_text(&A, buf, sizeof buf);
        printf("result: %s\n", buf);
    }
}

int main(int argc, char **argv)
{
    int i, ascii = 1, text = 0;
    const char *pbm = 0, *script = 0;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--pbm") == 0 && i + 1 < argc) pbm = argv[++i];
        else if (strcmp(argv[i], "--text") == 0) text = 1;
        else if (strcmp(argv[i], "--quiet") == 0) ascii = 0;
        else script = argv[i];
    }
    app_init(&A);
    app_tick(&A, 0);
    if (script) {
        const char *bad = 0;
        if (sim_run_keys(&A, script, &bad) < 0) { fprintf(stderr, "unknown key: %s\n", bad); return 1; }
        show(!ascii, pbm, text);
        return 0;
    }
    {
        char line[512];
        while (fgets(line, sizeof line, stdin)) {
            const char *bad = 0;
            if (sim_run_keys(&A, line, &bad) < 0) { printf("unknown key near: %s", bad); continue; }
            show(!ascii, pbm, text);
        }
    }
    return 0;
}
