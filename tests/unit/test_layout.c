/* Editor and natural-layout tests. */
#include <stdio.h>
#include <string.h>
#include "editor/editor.h"
#include "renderer/mathlay.h"
#include "parser/lexer.h"
#include "parser/tokinfo.h"
#include "calc/eval.h"
#include "calc/format.h"

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static void dump(const lcd_t *l, int y0, int y1, int x1)
{
    int x, y;
    for (y = y0; y < y1; y++) {
        for (x = 0; x < x1; x++) putchar(lcd_get(l, x, y) ? '#' : '.');
        putchar('\n');
    }
}

static void press(editor_t *e, const char *s)
{
    tok_t t[64];
    int n, i;
    lex_text(s, 0, t, 64, &n);
    for (i = 0; i < n; i++) ed_insert(e, t[i]);
}

static void test_struct_edit(void)
{
    editor_t e;
    tok_t b[ED_CAP + 24];
    int n;
    ed_clear(&e);
    ed_insert(&e, mk_tok(T_D1, 0));
    ed_insert_struct(&e, T_FRAC);           /* 1 over [] */
    CHECK(e.n == 4 && e.t[0].t == T_FRAC && e.t[1].t == T_D1 && e.t[2].t == T_FRACSEP && e.t[3].t == T_FRACEND, "frac from digit");
    CHECK(e.cur == 3, "cursor in denominator, %d", e.cur);
    ed_insert(&e, mk_tok(T_D4, 0));
    n = ed_build(&e, b, ED_CAP + 24);
    CHECK(n == 5, "build");
    {
        val_t v; ctx_t c; char s[64];
        ctx_init(&c);
        CHECK(calc_eval_tokens(&c, b, n, &v) == CE_OK, "eval frac");
        fmt_value_text(&c, &v, 1, 1, s, sizeof s);
        CHECK(strcmp(s, "1/4") == 0, "1/4 got %s", s);
    }
    /* DEL deletes digit, then moves out of the denominator, then removes the empty structure */
    ed_delete(&e);
    CHECK(e.n == 4, "del digit");
    ed_delete(&e);                          /* cursor after sep: moves left */
    CHECK(e.cur == 2, "move out of sep %d", e.cur);
    ed_vertical(&e, -1);
    ed_left(&e);
    CHECK(e.cur == 1 || e.cur == 0, "left");
}

static void test_autoclose(void)
{
    tok_t t[16], o[40];
    int n, m;
    lex_text("sin(30", 0, t, 16, &n);
    m = tok_autoclose(t, n, o, 40);
    CHECK(m == n, "lexer already closes: %d %d", m, n);
    /* unclosed function inside a fraction numerator closes before the separator */
    {
        tok_t u[8] = {{T_FRAC,0},{T_SIN,0},{T_D3,0},{T_FRACSEP,0},{T_D2,0},{T_FRACEND,0}};
        m = tok_autoclose(u, 6, o, 40);
        CHECK(m == 7 && o[3].t == T_RP && o[4].t == T_FRACSEP, "close before sep m=%d", m);
    }
}

static void test_layout(void)
{
    static lcd_t L;
    editor_t e;
    lay_cursor_t cc;
    box_t b;
    lcd_clear(&L);
    ed_clear(&e);
    press(&e, "frac(1,2)+sqrt(3)");
    ed_insert_struct(&e, T_POW);
    ed_insert(&e, mk_tok(T_D2, 0));
    ed_right(&e);
    b = lay_draw(&L, e.t, e.n, 2, 24, 0, e.cur, &cc);
    CHECK(b.w > 20 && b.asc >= 12 && b.desc >= 5, "box %d %d %d", b.w, b.asc, b.desc);
    CHECK(cc.found, "cursor found");
    dump(&L, 10, 28, 80);
    /* width is the same when measured alone */
    CHECK(lay_measure(e.t, e.n, 0).w == b.w, "measure == draw");
}

int main(void)
{
    test_struct_edit();
    test_autoclose();
    test_layout();
    printf("%d checks, %d failures\n", checks, fails);
    return fails != 0;
}
