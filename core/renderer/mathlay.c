#include "renderer/mathlay.h"
#include "parser/tokinfo.h"
#include <string.h>

typedef struct lay {
    lcd_t *l;              /* NULL while measuring */
    const tok_t *t;
    int n;
    int rec;               /* record the cursor position */
    int cur;
    lay_cursor_t *cc;
} lay_t;

#define MAX_LAY_DEPTH 10

static int imax(int a, int b) { return a > b ? a : b; }

static box_t seq(lay_t *L, int i0, int i1, int small, int fd, int dp, int x, int yb);

static box_t measure_seq(lay_t *L, int i0, int i1, int small, int fd, int dp)
{
    lcd_t *sl = L->l;
    int sr = L->rec;
    box_t b;
    L->l = 0;
    L->rec = 0;
    b = seq(L, i0, i1, small, fd, dp, 0, 0);
    L->l = sl;
    L->rec = sr;
    return b;
}

static void record(lay_t *L, int x, int yb, int small)
{
    if (!L->rec || !L->cc) return;
    L->cc->found = 1;
    L->cc->x = x - 1;
    L->cc->y = yb - (small ? SFONT_H : FONT_H);
    L->cc->h = (small ? SFONT_H : FONT_H) + 1;
}

/* Draw a label; \x01 .. \x02 delimits a raised small run. Returns width, raises *asc. */
static int label(lay_t *L, const char *s, int small, int x, int yb, int *asc)
{
    int cx = x, raised = 0;
    for (; *s; s++) {
        if (*s == '\x01') { raised = 1; continue; }
        if (*s == '\x02') { raised = 0; continue; }
        if (raised) {
            int r = small ? 3 : 4;
            if (L->l) lcd_char(L->l, cx, yb - r - SFONT_H, (unsigned char)*s, 1);
            *asc = imax(*asc, r + SFONT_H);
            cx += SFONT_W;
        } else if (small) {
            if (L->l) lcd_char(L->l, cx, yb - SFONT_H, (unsigned char)*s, 1);
            cx += SFONT_W;
        } else {
            if (L->l) lcd_char(L->l, cx, yb - FONT_H, (unsigned char)*s, 0);
            cx += FONT_W;
        }
    }
    return cx - x;
}

static int next_sep(const tok_t *t, int from, int e, uint8_t sep)
{
    int depth = 0, k;
    for (k = from; k < e; k++) {
        if (tok_struct_end(t[k].t)) depth++;
        else if (tok_is_struct_end(t[k].t)) depth--;
        else if (depth == 0 && t[k].t == sep) return k;
    }
    return e;
}

static void placeholder(lay_t *L, int small, int x, int yb)
{
    if (!L->l) return;
    if (small) lcd_frame(L->l, x, yb - SFONT_H, 3, SFONT_H);
    else lcd_frame(L->l, x, yb - FONT_H, 5, FONT_H);
}

/* stacked fraction of slots [a0,a1) over [b0,b1) at x; returns box */
static box_t fraction(lay_t *L, int a0, int a1, int b0, int b1, int small, int fd, int dp, int x, int yb)
{
    int cs = small || fd >= 1, ax = small ? 3 : 4;
    box_t nb = measure_seq(L, a0, a1, cs, fd + 1, dp + 1), db = measure_seq(L, b0, b1, cs, fd + 1, dp + 1), r;
    int w = imax(nb.w, db.w) + 2, axis = yb - ax;
    (void)seq(L, a0, a1, cs, fd + 1, dp + 1, x + 1 + (w - 2 - nb.w) / 2, axis - 1 - nb.desc);
    (void)seq(L, b0, b1, cs, fd + 1, dp + 1, x + 1 + (w - 2 - db.w) / 2, axis + 2 + db.asc);
    if (L->l) lcd_hline(L->l, x, axis, w, 1);
    r.w = w;
    r.asc = ax + 1 + nb.desc + nb.asc;
    r.desc = imax(0, 2 + db.asc + db.desc - ax);
    return r;
}

static void radical(lay_t *L, int x, int yb, int ytop, int rdesc, int rw, int small)
{
    int bv = yb + rdesc - 1, sw = small ? 4 : 5;
    if (!L->l) return;
    if (small) {
        lcd_pixel(L->l, x, bv - 1, 1);
        lcd_line(L->l, x + 1, bv, x + 3, ytop);
    } else {
        lcd_line(L->l, x, bv - 2, x + 2, bv);
        lcd_line(L->l, x + 2, bv, x + 4, ytop);
    }
    lcd_hline(L->l, x + sw - 1, ytop, rw + 3, 1);
}

static box_t structure(lay_t *L, int i, int e, int small, int fd, int dp, int x, int yb, int prev_asc)
{
    const tok_t *t = L->t;
    box_t r = {0, 0, 0};
    int sw = small ? 4 : 5;
    switch (t[i].t) {
    case T_FRAC: {
        int s = next_sep(t, i + 1, e, T_FRACSEP);
        r = fraction(L, i + 1, s, s + 1 < e ? s + 1 : e, e, small, fd, dp, x, yb);
        break;
    }
    case T_MFRAC: {
        int s1 = next_sep(t, i + 1, e, T_MSEP1), s2 = next_sep(t, s1 + 1 < e ? s1 + 1 : e, e, T_MSEP2);
        box_t wb = measure_seq(L, i + 1, s1, small, fd, dp + 1), fb;
        (void)seq(L, i + 1, s1, small, fd, dp + 1, x, yb);
        fb = fraction(L, s1 + 1 < e ? s1 + 1 : e, s2, s2 + 1 < e ? s2 + 1 : e, e, small, fd, dp, x + wb.w + 1, yb);
        r.w = wb.w + 1 + fb.w;
        r.asc = imax(wb.asc, fb.asc);
        r.desc = imax(wb.desc, fb.desc);
        break;
    }
    case T_SQRT: {
        box_t rb = measure_seq(L, i + 1, e, small, fd, dp + 1);
        int ytop = yb - rb.asc - 2;
        (void)seq(L, i + 1, e, small, fd, dp + 1, x + sw, yb);
        radical(L, x, yb, ytop, rb.desc, rb.w, small);
        r.w = sw + rb.w + 2;
        r.asc = rb.asc + 2;
        r.desc = rb.desc;
        break;
    }
    case T_ROOT: {
        int s = next_sep(t, i + 1, e, T_ROOTSEP);
        int a1 = s + 1 < e ? s + 1 : e;
        box_t ib = measure_seq(L, i + 1, s, 1, fd, dp + 1), rb = measure_seq(L, a1, e, small, fd, dp + 1);
        int ytop = yb - rb.asc - 2;
        (void)seq(L, i + 1, s, 1, fd, dp + 1, x, ytop + 1 + ib.asc);
        (void)seq(L, a1, e, small, fd, dp + 1, x + ib.w + sw, yb);
        radical(L, x + ib.w, yb, ytop, rb.desc, rb.w, small);
        r.w = ib.w + sw + rb.w + 2;
        r.asc = rb.asc + 2;
        r.desc = rb.desc;
        break;
    }
    default: { /* T_POW */
        int rs = imax(small ? 3 : 4, prev_asc - 3);
        box_t eb = measure_seq(L, i + 1, e, 1, fd, dp + 1);
        (void)seq(L, i + 1, e, 1, fd, dp + 1, x, yb - rs);
        r.w = eb.w + 1;
        r.asc = rs + eb.asc;
        r.desc = imax(0, eb.desc - rs);
        break;
    }
    }
    return r;
}

static box_t seq(lay_t *L, int i0, int i1, int small, int fd, int dp, int x, int yb)
{
    box_t b;
    int i = i0, cx = x, prev_asc;
    char buf[32];

    b.w = 0;
    b.asc = small ? SFONT_H : FONT_H;
    b.desc = 0;
    prev_asc = b.asc;
    if (dp > MAX_LAY_DEPTH) {
        b.w = small ? SFONT_W : FONT_W;
        return b;
    }
    if (dp > 0 && i0 >= i1) {
        if (L->rec && L->cur == i0) record(L, x, yb, small);
        placeholder(L, small, x, yb);
        b.w = small ? SFONT_W : FONT_W;
        return b;
    }
    for (;;) {
        uint8_t k;
        if (L->rec && L->cur == i) record(L, cx, yb, small);
        if (i >= i1) break;
        k = L->t[i].t;
        if (tok_struct_end(k)) {
            int e = tok_struct_match(L->t, L->n, i);
            if (e < i1) {
                box_t sb = structure(L, i, e, small, fd, dp, cx, yb, prev_asc);
                cx += sb.w;
                b.asc = imax(b.asc, sb.asc);
                b.desc = imax(b.desc, sb.desc);
                prev_asc = sb.asc;
                i = e + 1;
                continue;
            }
        }
        {
            const char *s = tok_label(L->t[i], buf, (int)sizeof buf);
            int pad = (k == T_ADD || k == T_SUB || k == T_MUL || k == T_DIV || k == T_EQ || k == T_STO) ? 1 : 0;
            int asc = 0, w;
            if (k == T_SQ || k == T_CUBE || k == T_RECIP || k == T_DEGSYM || k == T_RADSYM || k == T_GRADSYM)
                prev_asc = imax(prev_asc, 0);
            w = label(L, s, small, cx + pad, yb, &asc);
            cx += w + 2 * pad;
            b.asc = imax(b.asc, asc);
            prev_asc = imax(small ? SFONT_H : FONT_H, 0);
            if (asc > prev_asc) prev_asc = asc;
            i++;
        }
    }
    b.w = cx - x;
    return b;
}

box_t lay_measure(const tok_t *t, int n, int small)
{
    lay_t L;
    memset(&L, 0, sizeof L);
    L.t = t; L.n = n;
    return seq(&L, 0, n, small, 0, 0, 0, 0);
}

box_t lay_draw(lcd_t *l, const tok_t *t, int n, int x, int yb, int small, int cur, lay_cursor_t *cc)
{
    lay_t L;
    memset(&L, 0, sizeof L);
    L.l = l; L.t = t; L.n = n;
    L.cur = cur;
    L.cc = cc;
    L.rec = cur >= 0 && cc != 0;
    if (cc) cc->found = 0;
    return seq(&L, 0, n, small, 0, 0, x, yb);
}

int lay_cursor_x(const tok_t *t, int n, int small, int cur)
{
    lay_t L;
    lay_cursor_t cc;
    memset(&L, 0, sizeof L);
    memset(&cc, 0, sizeof cc);
    L.t = t; L.n = n; L.cur = cur; L.cc = &cc; L.rec = 1;
    (void)seq(&L, 0, n, small, 0, 0, 0, 0);
    return cc.found ? cc.x + 1 : 0;
}
