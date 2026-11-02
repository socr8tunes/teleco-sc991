#include "sim_keys.h"
#include "parser/tokinfo.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const char *const NAMES[K_COUNT] = {
    "SHIFT", "ALPHA", "LEFT", "UP", "DOWN", "RIGHT",
    "MENU", "MODE", "FRAC", "SQRT", "SQ", "POW",
    "LOG", "LN", "NEG", "DMS", "HYP", "SIN",
    "COS", "TAN", "STO", "SD", "ENG", "M+",
    "(", ")", "XINV", "FACT", "EE",
    "7", "8", "9", "DEL", "AC",
    "4", "5", "6", "*", "/",
    "1", "2", "3", "+", "-",
    "0", ".", "PI", "ANS", "EXE",
};

static const int DIGIT_KEY[10] = {K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9};

const char *sim_key_name(int key) { return key >= 0 && key < K_COUNT ? NAMES[key] : "?"; }

int sim_key_from_name(const char *name)
{
    int i;
    for (i = 0; i < K_COUNT; i++)
        if (strcmp(NAMES[i], name) == 0) return i;
    if (strcmp(name, "x") == 0) return K_MUL;
    if (strcmp(name, "=") == 0) return K_EXE;
    return -1;
}

int sim_run_keys(app_t *a, const char *s, const char **bad)
{
    int count = 0;
    char w[32];
    while (*s) {
        size_t n = 0;
        int k, allnum;
        while (*s && isspace((unsigned char)*s)) s++;
        if (!*s) break;
        while (s[n] && !isspace((unsigned char)s[n]) && n < sizeof w - 1) { w[n] = s[n]; n++; }
        w[n] = 0;
        if (bad) *bad = s;
        s += n;
        if (strncmp(w, "wait:", 5) == 0) { app_tick(a, a->now_ms + (uint32_t)atoi(w + 5)); continue; }
        if (w[0] == '{' && w[n - 1] == '}') { w[n - 1] = 0; if (ui_set_expr_text(a, w + 1)) return -1; continue; }   /* {expression text} loads the editor */
        allnum = 1;
        { size_t i; for (i = 0; i < n; i++) if (!isdigit((unsigned char)w[i]) && w[i] != '.') allnum = 0; }
        if (allnum && n > 1) {
            size_t i;
            for (i = 0; i < n; i++) { app_key(a, w[i] == '.' ? K_DOT : DIGIT_KEY[w[i] - '0']); count++; }
            continue;
        }
        k = sim_key_from_name(w);
        if (k < 0) return -1;
        app_key(a, k);
        count++;
    }
    return count;
}

static size_t put(char *buf, size_t cap, size_t pos, const char *s)
{
    size_t n = strlen(s);
    if (cap == 0) return 0;
    if (pos + n >= cap) n = cap - 1 - pos;
    memcpy(buf + pos, s, n);
    buf[pos + n] = 0;
    return pos + n;
}

size_t sim_result_text(const app_t *a, char *buf, size_t cap)
{
    val_t v;
    if (cap) buf[0] = 0;
    if (a->screen == SCR_GRID) return a->gr_msg ? put(buf, cap, 0, calc_err_name((calc_err_t)a->gr_msg)) : 0;
    if (!a->post_eval) return 0;
    if (!a->have_res) return put(buf, cap, 0, calc_err_name((calc_err_t)a->err));
    if (a->ctx.nres > 1) v = a->ctx.res[a->view_idx < a->ctx.nres ? a->view_idx : 0];
    else if (a->ctx.has_aux && a->view_idx == 1) val_real(&v, &a->ctx.aux);
    else v = a->res;
    {
        size_t n;
        ctx_fmt_saved_t sv = ctx_fmt_override(&a->ctx, a->eng_view ? FMT_ENG : -1, -1);
        if (a->dms_view && v.k == V_REAL) v.disp = 1;
        n = fmt_value_text(&a->ctx, &v, a->exact, 1, buf, cap);
        ctx_fmt_restore(&a->ctx, sv);
        return n;
    }
}

size_t sim_expr_text(const app_t *a, char *buf, size_t cap)
{
    size_t pos = 0;
    int i;
    char tmp[32];
    if (cap) buf[0] = 0;
    for (i = 0; i < a->ed.n; i++) {
        uint8_t tt = a->ed.t[i].t;
        const char *s = (tok_struct_end(tt) || tok_is_struct_end(tt) || tok_is_struct_sep(tt)) ? "" : tok_label(a->ed.t[i], tmp, (int)sizeof tmp);
        char one[2] = {0, 0};
        for (; *s; s++) {
            unsigned char c = (unsigned char)*s;
            if (c == 1 || c == 2) { one[0] = c == 1 ? '^' : ' '; if (c == 1) pos = put(buf, cap, pos, "^("); else pos = put(buf, cap, pos, ")"); continue; }
            if (c == 0x82) pos = put(buf, cap, pos, "*");
            else if (c == 0x83) pos = put(buf, cap, pos, "/");
            else if (c == 0x80) pos = put(buf, cap, pos, "sqrt");
            else if (c == 0x81) pos = put(buf, cap, pos, "pi");
            else if (c == 0x87) pos = put(buf, cap, pos, "sum");
            else if (c == 0x9C) pos = put(buf, cap, pos, "prod");
            else if (c >= 0x80) pos = put(buf, cap, pos, "?");
            else { one[0] = (char)c; pos = put(buf, cap, pos, one); }
        }
        if (tt == T_SQRT) pos = put(buf, cap, pos, "sqrt");
        else if (tt == T_ROOT) pos = put(buf, cap, pos, "root");
        else if (tt == T_POW) pos = put(buf, cap, pos, "^");
        else if (tt == T_MFRAC) pos = put(buf, cap, pos, "mfrac");
        if (tok_struct_end(tt)) pos = put(buf, cap, pos, "{");
        if (tok_is_struct_sep(a->ed.t[i].t)) pos = put(buf, cap, pos, "|");
        if (tok_is_struct_end(a->ed.t[i].t)) pos = put(buf, cap, pos, "}");
    }
    return pos;
}

void sim_frame_ascii(const lcd_t *l, FILE *out)
{
    int x, y;
    for (y = 0; y < LCD_H; y += 2) {
        for (x = 0; x < LCD_W; x++) {
            int t = lcd_get(l, x, y), b = lcd_get(l, x, y + 1);
            fputc(t && b ? '#' : t ? '\'' : b ? ',' : ' ', out);
        }
        fputc('\n', out);
    }
}

void sim_frame_pbm(const lcd_t *l, FILE *out)
{
    int x, y;
    fprintf(out, "P1\n%d %d\n", LCD_W, LCD_H);
    for (y = 0; y < LCD_H; y++) {
        for (x = 0; x < LCD_W; x++) fputc(lcd_get(l, x, y) ? '1' : '0', out);
        fputc('\n', out);
    }
}
