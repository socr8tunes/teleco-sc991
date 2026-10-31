/* Calculation screen, result rendering and startup logo. */
#include "ui/app.h"
#include "renderer/mathlay.h"
#include "parser/lexer.h"
#include "parser/tokinfo.h"
#include "calc/errors.h"
#include <string.h>

/* ---- result -> tokens (so results use the same natural layout as input) ---- */

typedef struct tb { tok_t *t; int cap, n, err; } tb_t;

static void put(tb_t *b, uint8_t t, uint8_t a)
{
    if (b->n >= b->cap) { b->err = 1; return; }
    b->t[b->n++] = mk_tok(t, a);
}

static void put_text(tb_t *b, const char *s)
{
    tok_t tmp[48];
    int n = 0, i;
    if (!s[0]) return;
    if (lex_text(s, 0, tmp, 48, &n) != CE_OK) { b->err = 1; return; }
    for (i = 0; i < n; i++) put(b, tmp[i].t, tmp[i].a);
}

static void put_frac(tb_t *b, const char *num, const char *den)
{
    put(b, T_FRAC, 0);
    put_text(b, num);
    put(b, T_FRACSEP, 0);
    put_text(b, den);
    put(b, T_FRACEND, 0);
}

static void put_part(tb_t *b, const fmt_part_t *p, int sign)
{
    const char *e;
    if (sign && p->neg) put(b, T_NEG, 0);
    switch (p->kind) {
    case FR_MIXED:
        put_text(b, p->whole);
        put_frac(b, p->num, p->den);
        break;
    case FR_FRAC:
    case FR_EXACT:
        if (p->den[0]) put_frac(b, p->num, p->den);
        else put_text(b, p->num);
        break;
    default:
        put_text(b, p->num);
        break;
    }
    e = p->exp;
    if (e[0]) {
        put(b, T_MUL, 0);
        put(b, T_D1, 0);
        put(b, T_D0, 0);
        put(b, T_POW, 0);
        for (; *e; e++) {
            if (*e == '-') put(b, T_NEG, 0);
            else if (*e >= '0' && *e <= '9') put(b, (uint8_t)(T_D0 + (*e - '0')), 0);
        }
        put(b, T_PEND, 0);
    }
}

int ui_res_tokens(const fmt_res_t *r, tok_t *out, int cap)
{
    tb_t b;
    b.t = out; b.cap = cap; b.n = 0; b.err = 0;
    if (r->kind != FR_CPLX) {
        put_part(&b, &r->p, 1);
    } else if (r->polar) {
        put_part(&b, &r->p, 1);
        put(&b, T_DANGLE, 0);
        put_part(&b, &r->q, 1);
    } else {
        int paren = r->q.kind == FR_EXACT && !r->q.den[0] && strchr(r->q.num, '+') != 0;
        if (!r->re_zero) {
            put_part(&b, &r->p, 1);
            put(&b, r->q.neg ? T_SUB : T_ADD, 0);
        } else if (r->q.neg) {
            put(&b, T_NEG, 0);
        }
        if (!r->im_one) {
            if (paren) put(&b, T_LP, 0);
            put_part(&b, &r->q, 0);
            if (paren) put(&b, T_RP, 0);
        }
        put(&b, T_I, 0);
    }
    return b.err ? -1 : b.n;
}

int ui_real_tokens(const ctx_t *c, const num_t *n, int exact, tok_t *out, int cap)
{
    fmt_res_t r;
    fmt_real(c, n, exact, &r);
    return ui_res_tokens(&r, out, cap);
}

/* UTF-8 text from the formatter -> LCD symbol codes */
void ui_u8lcd(const char *in, char *out, size_t cap)
{
    size_t o = 0;
    while (*in && o + 1 < cap) {
        unsigned char c = (unsigned char)*in;
        if (c == 0xE2 && (unsigned char)in[1] == 0x88 && (unsigned char)in[2] == 0x9A) { out[o++] = '\x80'; in += 3; }
        else if (c == 0xCF && (unsigned char)in[1] == 0x80) { out[o++] = '\x81'; in += 2; }
        else if (c == 0xC2 && (unsigned char)in[1] == 0xB0) { out[o++] = '\x84'; in += 2; }
        else if (c == 0xE2 && (unsigned char)in[1] == 0x88 && (unsigned char)in[2] == 0xA0) { out[o++] = '\x89'; in += 3; }
        else if (c == 0xE2 && (unsigned char)in[1] == 0x88 && (unsigned char)in[2] == 0x92) { out[o++] = '-'; in += 3; }
        else if (c == 0xC3 && (unsigned char)in[1] == 0x97) { out[o++] = '\x82'; in += 2; }
        else if (c == 0xC3 && (unsigned char)in[1] == 0xB7) { out[o++] = '\x83'; in += 2; }
        else if (c >= 0x80) { in++; }
        else { out[o++] = (char)c; in++; }
    }
    out[o] = 0;
}

void ui_title(lcd_t *l, const char *left, const char *right)
{
    lcd_fill(l, 0, 0, LCD_W, 8, 0);
    lcd_text(l, 2, 0, left, 0);
    if (right && right[0]) lcd_text_right(l, LCD_W - 2, 1, right, 1);
    lcd_invert(l, 0, 0, LCD_W, 8);
}

/* ---- status bar ------------------------------------------------------------ */

static int flag_box(lcd_t *l, int x, char ch)
{
    lcd_char(l, x + 2, 1, (unsigned char)ch, 1);
    lcd_invert(l, x, 0, 7, 7);
    return x + 9;
}

static void status_bar(const app_t *a, lcd_t *l)
{
    int x = 1;
    const ctx_t *c = &a->ctx;
    lcd_fill(l, 0, 0, LCD_W, 8, 0);
    if (a->shift) x = flag_box(l, x, 'S');
    if (a->alpha) x = flag_box(l, x, 'A');
    x = lcd_text(l, x, 1, c->angle == ANG_DEG ? "D" : c->angle == ANG_RAD ? "R" : "G", 1) + 3;
    if (c->base) x = lcd_text(l, x, 1, c->base == 2 ? "BIN" : c->base == 8 ? "OCT" : c->base == 10 ? "DEC" : "HEX", 1) + 3;
    else {
        const char *f = c->fmt == FMT_FIX ? "FIX" : c->fmt == FMT_SCI ? "SCI" : (c->fmt == FMT_ENG || a->eng_view) ? "ENG" : 0;
        if (f) x = lcd_text(l, x, 1, f, 1) + 3;
    }
    if (c->cplx) x = lcd_text(l, x, 1, c->polar ? "CPLX R<" : "CPLX", 1) + 3;
    if (c->stat.rows > 0) x = lcd_text(l, x, 1, "STAT", 1) + 3;
    if (!dec_is_zero(&c->var[VAR_M].re.v)) x = lcd_text(l, x, 1, "M", 1) + 3;
    if (!a->exact && a->have_res) (void)lcd_text(l, x, 1, "DEC", 1);
    if (a->battery_low) {
        lcd_text_right(l, LCD_W - 1, 1, "LOW BAT", 1);
    } else if (a->screen == SCR_CALC && (a->hist_pos >= 0 || (a->post_eval && ui_hist_count(&a->hist) > 1))) {
        lcd_text_right(l, LCD_W - 1, 1, "^", 1);
    }
    lcd_hline(l, 0, 8, LCD_W, 1);
}

/* ---- calculation screen ------------------------------------------------------ */

static void result_labels(const app_t *a, char *buf)
{
    const ctx_t *c = &a->ctx;
    buf[0] = 0;
    if (c->nres > 1) {
        static const char *const K1[] = {"x1=", "x2=", "x3="};
        static const char *const K2[] = {"x=", "y=", "z="};
        strcpy(buf, (c->res_kind == 2 ? K2 : K1)[a->view_idx < 3 ? a->view_idx : 0]);
    } else if (c->has_aux == 1) {
        strcpy(buf, a->view_idx == 0 ? "X=" : "L-R=");
    } else if (c->has_aux == 2) {
        if (a->is_pol) strcpy(buf, a->view_idx == 0 ? "r=" : "\x88=");
        else strcpy(buf, a->view_idx == 0 ? "x=" : "y=");
    }
}

static int current_value(const app_t *a, val_t *v)
{
    const ctx_t *c = &a->ctx;
    if (c->nres > 1) { *v = c->res[a->view_idx < c->nres ? a->view_idx : 0]; return 1; }
    if (c->has_aux && a->view_idx == 1) { val_real(v, &c->aux); return 1; }
    *v = a->res;
    return 1;
}

static void draw_result(const app_t *a, lcd_t *l, int top)
{
    val_t v;
    char label[8];
    int x_right = LCD_W - 3;
    if (!a->have_res) {
        const char *m = calc_err_name((calc_err_t)a->err);
        lcd_text_right(l, x_right, LCD_H - 10, m, 0);
        lcd_text(l, 3, LCD_H - 6, "[AC]:Cancel [<][>]:Goto", 1);
        return;
    }
    current_value(a, &v);
    result_labels(a, label);
    if (label[0]) lcd_text(l, 3, LCD_H - 10, label, 0);
    if (a->ctx.base || v.k == V_MAT || v.k == V_VCT || (v.k == V_REAL && (v.disp == 1 || a->dms_view))) {
        char t[96], u[96];
        if (v.k == V_MAT || v.k == V_VCT) {
            lcd_text_right(l, x_right, LCD_H - 10, v.k == V_MAT ? "Matrix  [=] view" : "Vector  [=] view", 0);
            return;
        }
        if (a->dms_view) v.disp = 1;
        fmt_value_text(&a->ctx, &v, a->exact, 0, t, sizeof t);
        ui_u8lcd(t, u, sizeof u);
        lcd_text_right(l, x_right, LCD_H - 10, u, 0);
    } else {
        tok_t tk[96];
        ctx_t cc = a->ctx;
        fmt_res_t r;
        int n;
        box_t b;
        if (a->eng_view) cc.fmt = FMT_ENG;
        fmt_value(&cc, &v, a->exact, &r);
        n = ui_res_tokens(&r, tk, 96);
        if (n < 0) { lcd_text_right(l, x_right, LCD_H - 10, "Display ERROR", 0); return; }
        b = lay_measure(tk, n, 0);
        lcd_clip(l, 0, top, LCD_W, LCD_H - top);
        (void)lay_draw(l, tk, n, b.w + 6 > LCD_W ? 3 : x_right - b.w, LCD_H - 3 - b.desc, 0, -1, 0);
        lcd_unclip(l);
    }
}

static void calc_screen(const app_t *a, lcd_t *l)
{
    const editor_t *e = &a->ed;
    box_t b = lay_measure(e->t, e->n, 0);
    lay_cursor_t cc;
    int yb = 10 + (b.asc > FONT_H ? b.asc : FONT_H), cx, xoff = 0, rtop = LCD_H - 12;
    status_bar(a, l);
    if (a->have_res || a->err) {
        val_t v;
        if (a->have_res && a->ctx.base == 0) {
            tok_t tk[96];
            fmt_res_t r;
            ctx_t cc2 = a->ctx;
            int n;
            current_value(a, &v);
            if (v.k != V_MAT && v.k != V_VCT && !(v.k == V_REAL && v.disp == 1)) {
                fmt_value(&cc2, &v, a->exact, &r);
                n = ui_res_tokens(&r, tk, 96);
                if (n >= 0) rtop = LCD_H - 3 - (lay_measure(tk, n, 0).asc + lay_measure(tk, n, 0).desc);
            }
        }
    }
    cx = lay_cursor_x(e->t, e->n, 0, e->cur);
    if (b.w + 8 > LCD_W) {
        xoff = cx - 120;
        if (xoff > b.w + 8 - LCD_W) xoff = b.w + 8 - LCD_W;
        if (xoff < 0) xoff = 0;
    }
    lcd_clip(l, 0, 9, LCD_W, rtop - 10 > 10 ? rtop - 10 : 10);
    (void)lay_draw(l, e->t, e->n, 4 - xoff, yb, 0, e->cur, &cc);
    if (!a->post_eval && !a->blink) {
        if (cc.found) lcd_vline(l, cc.x, cc.y, cc.h, 1);
        else lcd_vline(l, 3, yb - FONT_H, FONT_H + 1, 1);
    }
    lcd_unclip(l);
    if (xoff > 0) lcd_text(l, 0, yb - FONT_H, S_LEFT, 0);
    if (xoff + LCD_W < b.w + 8) lcd_text(l, LCD_W - 6, yb - FONT_H, S_RIGHT, 0);
    if (a->post_eval) draw_result(a, l, rtop);
    else if (a->hist_pos >= 0) lcd_text(l, 3, LCD_H - 6, "History  [=] run", 1);
}

/* ---- startup logo --------------------------------------------------------------- */

static void big_char(lcd_t *l, int x, int y, char ch, int sc)
{
    lcd_t t;
    int cx, ry, sx, sy;
    lcd_clear(&t);
    lcd_char(&t, 0, 0, (unsigned char)ch, 0);
    for (cx = 0; cx < 5; cx++)
        for (ry = 0; ry < 7; ry++)
            if (lcd_get(&t, cx, ry))
                for (sx = 0; sx < sc; sx++)
                    for (sy = 0; sy < sc; sy++) lcd_pixel(l, x + cx * sc + sx, y + ry * sc + sy, 1);
}

void ui_logo_render(const app_t *a, lcd_t *l)
{
    static const char NAME[] = "TELECO";
    int i, x0 = (LCD_W - (6 * 5 * 3 + 5 * 3 + 5 * 3 - 6)) / 2, sc = 3;
    (void)a;
    lcd_clear(l);
    /* wordmark: each letter 15 px wide with 3 px gaps (scaled from the 5x7 font) */
    x0 = (LCD_W - (6 * 15 + 5 * 3)) / 2;
    for (i = 0; i < 6; i++) big_char(l, x0 + i * 18, 14, NAME[i], sc);
    /* solar-cell strip under the wordmark: four cells with the model name */
    {
        int y = 42, cw = 12, gx = (LCD_W - (4 * cw + 3 * 2)) / 2;
        for (i = 0; i < 4; i++) {
            lcd_frame(l, gx + i * (cw + 2), y, cw, 6);
            lcd_vline(l, gx + i * (cw + 2) + cw / 2, y, 6, 1);
        }
        lcd_text(l, (LCD_W - lcd_text_w("SC991", 0)) / 2, 52, "SC991", 0);
    }
}

void app_render(const app_t *a, lcd_t *l)
{
    lcd_clear(l);
    switch (a->screen) {
    case SCR_LOGO: ui_logo_render(a, l); break;
    case SCR_CALC: calc_screen(a, l); break;
    case SCR_MENU: ui_menu_render(a, l); break;
    case SCR_STATEDIT: ui_stat_render(a, l); break;
    case SCR_STATRES: ui_statres_render(a, l); break;
    case SCR_MATVIEW: ui_matview_render(a, l); break;
    case SCR_CONFIRM: ui_confirm_render(a, l); break;
    default: break;
    }
}
