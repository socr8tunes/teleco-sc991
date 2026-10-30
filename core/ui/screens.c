/* Statistics editor / results, matrix viewer and the confirmation dialog. */
#include "ui/app.h"
#include "renderer/mathlay.h"
#include <stdio.h>
#include <string.h>

/* ---- statistics data editor ----------------------------------------------- */

#define ST_VIS 5

static int st_cols(const app_t *a) { return a->ctx.stat.type == ST_1VAR ? 2 : 3; }
/* field of a column: 0 = x, 1 = y, 2 = frequency */
static int st_field(const app_t *a, int col) { return (a->ctx.stat.type == ST_1VAR && col == 1) ? 2 : col; }

static void cell_text(const app_t *a, int row, int col, char *buf, size_t cap)
{
    const stat_t *s = &a->ctx.stat;
    int f = st_field(a, col);
    buf[0] = 0;
    if (row >= s->rows) return;
    if (f == 2) { snprintf(buf, cap, "%d", s->f[row]); return; }
    {
        num_t n;
        val_t v;
        char t[64];
        num_from_dec(&n, f == 0 ? &s->x[row] : &s->y[row]);
        val_real(&v, &n);
        fmt_value_text(&a->ctx, &v, 0, 1, t, sizeof t);
        snprintf(buf, cap, "%s", t);
    }
}

static const char *stat_type_name(int t)
{
    static const char *const N[] = {"1-Var", "A+BX", "A+BX+CX2", "A+B LN X", "A E^BX", "A B^X", "A X^B", "A+B/X"};
    return N[t & 7];
}

static void commit_cell(app_t *a)
{
    stat_t *s = &a->ctx.stat;
    int f = st_field(a, a->st_col);
    dec_t d;
    a->st_buf[a->st_len] = 0;
    if (a->st_len == 0 || dec_from_str(&d, a->st_buf) != DEC_OK) { a->beep = 1; return; }
    if (f == 2) {
        int64_t k;
        if (dec_to_i64(&d, &k) || k < 1 || k > 99) { a->beep = 1; return; }
        if (a->st_row >= s->rows) { a->beep = 1; return; }
        s->f[a->st_row] = (uint8_t)k;
    } else {
        if (a->st_row >= s->rows) {
            dec_t zero;
            dec_zero(&zero);
            if (stat_add(s, &zero, &zero, 1) != CE_OK) { a->beep = 1; return; }
        }
        if (f == 0) s->x[a->st_row] = d; else s->y[a->st_row] = d;
    }
    a->st_len = 0;
    a->st_editing = 0;
    if (a->st_row < STAT_ROWS - 1 && a->st_row <= s->rows - 1 + 1) a->st_row++;
    if (a->st_row > s->rows) a->st_row = (uint8_t)s->rows;
}

static void st_scroll(app_t *a)
{
    if (a->st_row < a->st_top) a->st_top = a->st_row;
    if (a->st_row >= a->st_top + ST_VIS) a->st_top = (uint8_t)(a->st_row - ST_VIS + 1);
}

void ui_stat_key(app_t *a, int key)
{
    stat_t *s = &a->ctx.stat;
    int nc = st_cols(a);
    char ch = 0;
    switch (key) {
    case K_UP: a->st_editing = 0; a->st_len = 0; if (a->st_row > 0) a->st_row--; break;
    case K_DOWN: a->st_editing = 0; a->st_len = 0; if (a->st_row < s->rows && a->st_row < STAT_ROWS - 1) a->st_row++; break;
    case K_LEFT: a->st_editing = 0; a->st_len = 0; if (a->st_col > 0) a->st_col--; break;
    case K_RIGHT: a->st_editing = 0; a->st_len = 0; if (a->st_col + 1 < nc) a->st_col++; break;
    case K_EXE:
        if (a->st_editing) commit_cell(a);
        else if (a->st_row < s->rows) a->st_row++;
        break;
    case K_DEL:
        if (a->st_editing && a->st_len > 0) { a->st_len--; if (a->st_len == 0) a->st_editing = 0; }
        else if (a->st_row < s->rows) (void)stat_delete(s, a->st_row);
        break;
    case K_AC:
        if (a->st_editing) { a->st_editing = 0; a->st_len = 0; }
        else a->screen = SCR_CALC;
        break;
    case K_MENU: ui_menu_open(a, MN_STAT); break;
    case K_MODE: ui_menu_open(a, MN_STATTYPE); break;
    case K_0: ch = '0'; break; case K_1: ch = '1'; break; case K_2: ch = '2'; break; case K_3: ch = '3'; break;
    case K_4: ch = '4'; break; case K_5: ch = '5'; break; case K_6: ch = '6'; break; case K_7: ch = '7'; break;
    case K_8: ch = '8'; break; case K_9: ch = '9'; break;
    case K_DOT: ch = '.'; break;
    case K_NEG: case K_SUB: ch = '-'; break;
    case K_EE: ch = 'E'; break;
    default: break;
    }
    if (ch) {
        if (a->st_len < (int)sizeof a->st_buf - 1) { a->st_buf[a->st_len++] = ch; a->st_editing = 1; }
        else a->beep = 1;
    }
    st_scroll(a);
}

void ui_stat_render(const app_t *a, lcd_t *l)
{
    const stat_t *s = &a->ctx.stat;
    int nc = st_cols(a), i, c, colw = nc == 2 ? 84 : 56;
    char buf[40];
    static const char *const H1[] = {"X", "FREQ"}, *const H2[] = {"X", "Y", "FREQ"};
    lcd_fill(l, 0, 0, LCD_W, 8, 1);
    snprintf(buf, sizeof buf, "STAT  %s", stat_type_name(s->type));
    lcd_text(l, 2, 0, buf, 0);
    lcd_invert(l, 2, 0, lcd_text_w(buf, 0), 8);
    snprintf(buf, sizeof buf, "n=%d", s->rows);
    lcd_text_right(l, LCD_W - 2, 1, buf, 1);
    lcd_invert(l, LCD_W - 2 - lcd_text_w(buf, 1), 1, lcd_text_w(buf, 1), 5);
    for (c = 0; c < nc; c++) lcd_text(l, 18 + c * colw, 10, (nc == 2 ? H1 : H2)[c], 0);
    lcd_hline(l, 0, 18, LCD_W, 1);
    for (i = 0; i < ST_VIS; i++) {
        int row = a->st_top + i, y = 20 + i * 8;
        snprintf(buf, sizeof buf, "%2d", row + 1);
        lcd_text(l, 1, y, buf, 1);
        for (c = 0; c < nc; c++) {
            int x = 18 + c * colw, cur = (row == a->st_row && c == a->st_col);
            if (cur && a->st_editing) { memcpy(buf, a->st_buf, (size_t)a->st_len); buf[a->st_len] = 0; }
            else cell_text(a, row, c, buf, sizeof buf);
            if ((int)strlen(buf) > (colw - 4) / FONT_W) buf[(colw - 4) / FONT_W] = 0;
            lcd_text(l, x, y, buf, 0);
            if (cur) lcd_invert(l, x - 2, y - 1, colw - 4, 9);
        }
    }
    if (a->st_top > 0) lcd_text(l, LCD_W - 6, 20, S_UP, 0);
    if (a->st_top + ST_VIS <= s->rows) lcd_text(l, LCD_W - 6, 52, S_DOWN, 0);
    lcd_text(l, 2, 59, "[=]set [DEL]del row [MENU]", 1);
}

/* ---- statistics results ------------------------------------------------------ */

static const char *const SV_TITLE[SV_COUNT] = {
    "n", "\x87x", "\x87x2", "x-bar", "\x95x", "sx", "\x87y", "\x87y2", "y-bar", "\x95y", "sy", "\x87xy",
    "a", "b", "c", "r", "minX", "maxX", "minY", "maxY", "Q1", "Med", "Q3", "\x87x3", "\x87x4", "\x87x2y",
};

/* valid variable ids in display order */
static int sr_list(const app_t *a, uint8_t *ids)
{
    int i, n = 0;
    num_t v;
    for (i = 0; i < SV_COUNT; i++)
        if (stat_var(&a->ctx.stat, i, a->ctx.angle, &v) == CE_OK) ids[n++] = (uint8_t)i;
    return n;
}

void ui_statres_key(app_t *a, int key)
{
    uint8_t ids[SV_COUNT];
    int n = sr_list(a, ids);
    switch (key) {
    case K_UP: if (a->sr_page > 0) a->sr_page--; break;
    case K_DOWN: if (a->sr_page + 1 < n) a->sr_page++; break;
    case K_EXE:
        if (n > 0) {
            a->screen = SCR_CALC;
            ui_insert_menu_tok(a, T_STATVAR, ids[a->sr_page]);
        }
        break;
    case K_AC: case K_LEFT: a->screen = SCR_CALC; break;
    case K_MENU: ui_menu_open(a, MN_STAT); break;
    default: break;
    }
}

void ui_statres_render(const app_t *a, lcd_t *l)
{
    uint8_t ids[SV_COUNT];
    int n = sr_list(a, ids), top, i, rows = 6;
    char buf[64], u[64];
    lcd_fill(l, 0, 0, LCD_W, 8, 1);
    lcd_text(l, 2, 0, "STAT RESULT", 0);
    lcd_invert(l, 2, 0, lcd_text_w("STAT RESULT", 0), 8);
    if (n == 0) { lcd_text(l, 4, 24, "No data. Use MENU > Statistics", 1); return; }
    top = a->sr_page - rows / 2;
    if (top > n - rows) top = n - rows;
    if (top < 0) top = 0;
    for (i = 0; i < rows && top + i < n; i++) {
        num_t v;
        val_t vv;
        int y = 10 + i * 9, id = ids[top + i];
        (void)stat_var(&a->ctx.stat, id, a->ctx.angle, &v);
        val_real(&vv, &v);
        fmt_value_text(&a->ctx, &vv, 0, 0, buf, sizeof buf);
        ui_u8lcd(buf, u, sizeof u);
        lcd_text(l, 3, y, SV_TITLE[id], 0);
        lcd_text(l, 3 + 8 * FONT_W, y, "=", 0);
        lcd_text_right(l, LCD_W - 8, y, u, 0);
        if (top + i == a->sr_page) lcd_invert(l, 0, y - 1, LCD_W - 8, 9);
    }
}

/* ---- matrix / vector viewer ------------------------------------------------------ */

#define MV_CELL_TOKS 48

static int mv_dims(const app_t *a, int *rows, int *cols, const num_t **e)
{
    const val_t *v = &a->res;
    if (v->k == V_MAT) { *rows = a->ctx.mats[v->mat].r; *cols = a->ctx.mats[v->mat].c; *e = a->ctx.mats[v->mat].e; return 1; }
    if (v->k == V_VCT) { *rows = a->ctx.vcts[v->mat].r; *cols = 1; *e = a->ctx.vcts[v->mat].e; return 1; }
    return 0;
}

void ui_matview_key(app_t *a, int key)
{
    int rows, cols;
    const num_t *e;
    if (!mv_dims(a, &rows, &cols, &e)) { a->screen = SCR_CALC; return; }
    switch (key) {
    case K_UP: if (a->mv_r > 0) a->mv_r--; break;
    case K_DOWN: if (a->mv_r + 1 < rows) a->mv_r++; break;
    case K_LEFT: if (a->mv_c > 0) a->mv_c--; break;
    case K_RIGHT: if (a->mv_c + 1 < cols) a->mv_c++; break;
    case K_AC: case K_EXE: a->screen = SCR_CALC; break;
    default: break;
    }
}

void ui_matview_render(const app_t *a, lcd_t *l)
{
    int rows, cols, i, j, x, y, ytop = 11, ybot = LCD_H - 2;
    const num_t *e;
    ctx_t cc = a->ctx;
    char buf[24];
    if (!mv_dims(a, &rows, &cols, &e)) return;
    cc.frac_improper = 1;
    lcd_fill(l, 0, 0, LCD_W, 8, 1);
    snprintf(buf, sizeof buf, "%s %dx%d", a->res.k == V_MAT ? "MATRIX" : "VECTOR", rows, cols);
    lcd_text(l, 2, 0, buf, 0);
    lcd_invert(l, 2, 0, lcd_text_w(buf, 0), 8);
    x = 8;
    {
        int colw[MAT_MAX], rowa[MAT_MAX], rowd[MAT_MAX], vis_rows = 0, tot_h = 0, ctot = 0;
        tok_t tk[MV_CELL_TOKS];
        for (j = 0; j < MAT_MAX; j++) { colw[j] = 0; rowa[j] = 0; rowd[j] = 0; }
        for (i = a->mv_r; i < rows; i++) {
            for (j = a->mv_c; j < cols; j++) {
                int n = ui_real_tokens(&cc, &e[i * cols + j], a->exact, tk, MV_CELL_TOKS);
                box_t b = n >= 0 ? lay_measure(tk, n, 0) : (box_t){6, 7, 0};
                if (b.w > colw[j]) colw[j] = b.w;
                if (b.asc > rowa[i]) rowa[i] = b.asc;
                if (b.desc > rowd[i]) rowd[i] = b.desc;
            }
        }
        for (i = a->mv_r; i < rows; i++) {
            int h = rowa[i] + rowd[i] + 4;
            if (tot_h + h > ybot - ytop) break;
            tot_h += h;
            vis_rows++;
        }
        if (vis_rows == 0) vis_rows = 1;
        for (j = a->mv_c; j < cols; j++) { if (ctot + colw[j] + 8 > LCD_W - 20) break; ctot += colw[j] + 8; }
        y = ytop + 2;
        for (i = a->mv_r; i < a->mv_r + vis_rows && i < rows; i++) {
            int xx = x + 4;
            for (j = a->mv_c; j < cols; j++) {
                int n;
                if (xx + colw[j] > LCD_W - 10) break;
                n = ui_real_tokens(&cc, &e[i * cols + j], a->exact, tk, MV_CELL_TOKS);
                if (n >= 0) {
                    box_t b = lay_measure(tk, n, 0);
                    (void)lay_draw(l, tk, n, xx + (colw[j] - b.w) / 2, y + rowa[i], 0, -1, 0);
                }
                xx += colw[j] + 8;
            }
            y += rowa[i] + rowd[i] + 4;
        }
        /* brackets */
        {
            int xr = x + 4 + ctot - 4 + 2, y0 = ytop, y1 = y;
            lcd_vline(l, x, y0, y1 - y0, 1); lcd_hline(l, x, y0, 3, 1); lcd_hline(l, x, y1 - 1, 3, 1);
            lcd_vline(l, xr, y0, y1 - y0, 1); lcd_hline(l, xr - 2, y0, 3, 1); lcd_hline(l, xr - 2, y1 - 1, 3, 1);
        }
        if (a->mv_r > 0) lcd_text(l, LCD_W - 6, 10, S_UP, 0);
        if (a->mv_r + vis_rows < rows) lcd_text(l, LCD_W - 6, 56, S_DOWN, 0);
    }
}

/* ---- confirmation dialog ----------------------------------------------------------- */

void ui_confirm_key(app_t *a, int key)
{
    if (key == K_EXE) {
        if (a->confirm_action == 1) ui_reset_memory(a);
        a->screen = SCR_CALC;
    } else if (key == K_AC) {
        a->screen = SCR_CALC;
    }
}

void ui_confirm_render(const app_t *a, lcd_t *l)
{
    (void)a;
    lcd_fill(l, 0, 0, LCD_W, 8, 1);
    lcd_text(l, 2, 0, "RESET", 0);
    lcd_invert(l, 2, 0, lcd_text_w("RESET", 0), 8);
    lcd_text(l, 6, 16, "Clear ALL memory?", 0);
    lcd_text(l, 6, 28, "Variables, Ans, history,", 0);
    lcd_text(l, 6, 38, "statistics data, settings.", 0);
    lcd_text(l, 6, 52, "[=] Yes      [AC] No", 0);
}
