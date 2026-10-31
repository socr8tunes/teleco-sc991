/* Random key fuzzing of the whole UI: no crashes, editor invariants hold, every screen renders. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_keys.h"
#include "parser/tokinfo.h"

static app_t A;
static lcd_t L;
static uint32_t rng;
static uint32_t rnd(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

static int balanced(const editor_t *e)
{
    int depth = 0, i;
    for (i = 0; i < e->n; i++) {
        if (tok_struct_end(e->t[i].t)) depth++;
        else if (tok_is_struct_end(e->t[i].t)) { if (--depth < 0) return 0; }
    }
    return depth == 0;
}

int main(int argc, char **argv)
{
    int seeds = argc > 1 ? atoi(argv[1]) : 20, keys = argc > 2 ? atoi(argv[2]) : 20000, s;
    long total = 0;
    for (s = 1; s <= seeds; s++) {
        int k;
        rng = 2463534242u * (uint32_t)s + 1u;
        app_init(&A);
        app_tick(&A, 5000);
        for (k = 0; k < keys; k++) {
            int key = (int)(rnd() % K_COUNT);
            /* keep the run from sitting in "off" or confirm dialogs forever */
            if (A.screen == SCR_OFF && (rnd() & 7) == 0) key = K_AC;
            app_key(&A, key);
            if ((rnd() & 63) == 0) app_tick(&A, A.now_ms + 700);
            total++;
            if (!balanced(&A.ed) || A.ed.cur < 0 || A.ed.cur > A.ed.n || A.ed.n > ED_CAP) {
                char ex[160];
                sim_expr_text(&A, ex, sizeof ex);
                printf("seed %d key %d: editor invariant broken (%s) n=%d cur=%d\n", s, k, ex, A.ed.n, A.ed.cur);
                return 1;
            }
            if (A.menu_sp > MENU_DEPTH) { printf("menu overflow\n"); return 1; }
            if ((k & 7) == 0) app_render(&A, &L);
        }
        app_render(&A, &L);
    }
    printf("fuzz: %ld keys over %d seeds, no failures\n", total, seeds);
    return 0;
}
