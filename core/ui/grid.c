/*
 * Coefficient grid: guided entry of the numbers of a linear system (2, 3 or 4 unknowns), one equation per
 * row. Every cell can be revisited and corrected before solving. The grid holds numbers only (no formulas);
 * it lives in the application state, is never saved to retained memory, and is cleared by RESET and power loss.
 */
#include "ui/app.h"
#include "solvers/eqn.h"
#include <stdio.h>
#include <string.h>

#define GR_MAXW 20

static int gr_rows(const app_t *a) { return a->gr_n; }
static int gr_cols(const app_t *a) { return a->gr_n + 1; }
static const char UNK[] = "xyzw";

void ui_grid_open(app_t *a, int n)
{
    if (n < 2 || n > 4) n = 2;
    if (a->gr_n != n) { memset(a->gr_val, 0, sizeof a->gr_val); a->gr_msg = 0; }
    a->gr_n = (uint8_t)n;
    a->gr_row = a->gr_col = 0;
    a->gr_editing = 0; a->gr_len = 0;
    a->menu_sp = 0;
    a->screen = SCR_GRID;
}

static void gr_commit(app_t *a)
{
    dec_t d;
    int idx = a->gr_row * 5 + a->gr_col;
    a->gr_buf[a->gr_len] = 0;
    if (a->gr_len == 0 || dec_from_str(&d, a->gr_buf) != DEC_OK) { a->beep = 1; return; }
    a->gr_val[idx] = d;
    a->gr_editing = 0; a->gr_len = 0;
    a->gr_msg = 0;
    if (a->gr_col + 1 < gr_cols(a)) a->gr_col++;
    else if (a->gr_row + 1 < gr_rows(a)) { a->gr_row++; a->gr_col = 0; }
}

static void gr_solve(app_t *a)
{
    int n = a->gr_n, i, j;
    num_t co[GR_MAXW], x[4];
    calc_err_t er;
    memset(co, 0, sizeof co);
    for (i = 0; i < n; i++)
        for (j = 0; j <= n; j++) num_from_dec(&co[i * (n + 1) + j], &a->gr_val[i * 5 + j]);
    er = eqn_simul(n, co, x);
    if (er) { a->gr_msg = (uint8_t)er; a->beep = 1; return; }
    for (i = 0; i < n; i++) val_real(&a->ctx.res[i], &x[i]);
    a->ctx.nres = (uint8_t)n;
    a->ctx.res_kind = 2;
    a->ctx.has_aux = 0;
    a->res = a->ctx.res[0];
    ed_clear(&a->ed);
    a->post_eval = 1; a->have_res = 1; a->err = 0;
    a->view_idx = 0; a->view_cnt = (uint8_t)n; a->hist_pos = -1;
    a->dms_view = a->eng_view = 0;
    a->screen = SCR_CALC;
}

void ui_grid_key(app_t *a, int key)
{
    char ch = 0;
    switch (key) {
    case K_UP: a->gr_editing = 0; a->gr_len = 0; if (a->gr_row > 0) a->gr_row--; break;
    case K_DOWN: a->gr_editing = 0; a->gr_len = 0; if (a->gr_row + 1 < gr_rows(a)) a->gr_row++; break;
    case K_LEFT: a->gr_editing = 0; a->gr_len = 0; if (a->gr_col > 0) a->gr_col--; break;
    case K_RIGHT: a->gr_editing = 0; a->gr_len = 0; if (a->gr_col + 1 < gr_cols(a)) a->gr_col++; break;
    case K_EXE:
        if (a->gr_editing) gr_commit(a);
        else gr_solve(a);
        break;
    case K_DEL:
        if (a->gr_editing && a->gr_len > 0) { a->gr_len--; if (a->gr_len == 0) a->gr_editing = 0; }
        else { dec_zero(&a->gr_val[a->gr_row * 5 + a->gr_col]); a->gr_msg = 0; }
        break;
    case K_AC:
        if (a->gr_editing) { a->gr_editing = 0; a->gr_len = 0; }
        else a->screen = SCR_CALC;
        break;
    case K_0: ch = '0'; break; case K_1: ch = '1'; break; case K_2: ch = '2'; break; case K_3: ch = '3'; break;
    case K_4: ch = '4'; break; case K_5: ch = '5'; break; case K_6: ch = '6'; break; case K_7: ch = '7'; break;
    case K_8: ch = '8'; break; case K_9: ch = '9'; break;
    case K_DOT: ch = '.'; break;
    case K_NEG: case K_SUB: ch = '-'; break;
    case K_EE: ch = 'E'; break;
    default: break;
    }
    if (ch) {
        if (a->gr_len < (int)sizeof a->gr_buf - 1) { a->gr_buf[a->gr_len++] = ch; a->gr_editing = 1; a->gr_msg = 0; }
        else a->beep = 1;
    }
}

static void cell_str(const app_t *a, int r, int c, char *buf, size_t cap)
{
    num_t n;
    val_t v;
    num_from_dec(&n, &a->gr_val[r * 5 + c]);
    val_real(&v, &n);
    fmt_value_text(&a->ctx, &v, 0, 1, buf, cap);
}

void ui_grid_render(const app_t *a, lcd_t *l)
{
    int n = a->gr_n, nc = n + 1, r, c;
    char buf[64], t[40];
    snprintf(buf, sizeof buf, "SIMUL %d", n);
    snprintf(t, sizeof t, "eq %d", a->gr_row + 1);
    ui_title(l, buf, t);
    for (c = 0; c < n; c++) { buf[0] = UNK[c]; buf[1] = 0; lcd_text(l, 12 + c * 35 + 12, 10, buf, 0); }
    lcd_text(l, 12 + n * 35 + 12, 10, "=", 0);
    lcd_hline(l, 0, 18, LCD_W, 1);
    for (r = 0; r < n; r++) {
        int y = 20 + r * 8;
        snprintf(buf, sizeof buf, "%d", r + 1);
        lcd_text(l, 2, y, buf, 1);
        for (c = 0; c < nc; c++) {
            int x = 11 + c * 35, cur = (r == a->gr_row && c == a->gr_col);
            if (cur && a->gr_editing) { memcpy(buf, a->gr_buf, (size_t)a->gr_len); buf[a->gr_len] = 0; }
            else cell_str(a, r, c, buf, sizeof buf);
            if ((int)strlen(buf) > 5) { buf[5] = 0; buf[4] = '~'; }      /* '~' marks a longer value (shown below) */
            lcd_text(l, x + 1, y, buf, 0);
            if (cur) lcd_invert(l, x - 1, y - 1, 34, 9);
        }
    }
    /* what is being entered, and the full value of the selected cell */
    {
        char full[24];
        if (a->gr_editing) { memcpy(full, a->gr_buf, (size_t)a->gr_len); full[a->gr_len] = 0; }
        else cell_str(a, a->gr_row, a->gr_col, full, sizeof full);
        if (a->gr_col < n) snprintf(buf, sizeof buf, "Eq%d: coef of %c = %s", a->gr_row + 1, UNK[a->gr_col], full);
        else snprintf(buf, sizeof buf, "Eq%d: constant = %s", a->gr_row + 1, full);
        lcd_text(l, 2, 53, buf, 1);
    }
    if (a->gr_msg) {
        lcd_fill(l, 0, 56, LCD_W, 8, 1);
        lcd_text(l, 2, 57, calc_err_name((calc_err_t)a->gr_msg), 1);
        lcd_invert(l, 0, 56, LCD_W, 8);
        lcd_text(l, 2, 57, calc_err_name((calc_err_t)a->gr_msg), 0);
    } else {
        lcd_text(l, 2, 59, "[=]set/solve [DEL]0/back [AC]exit", 1);
    }
}
