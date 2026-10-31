/* List menus: function catalogue, constants, conversions and the mode setup. */
#include "ui/app.h"
#include "units/units.h"
#include "parser/tokinfo.h"
#include <string.h>
#include <stdio.h>

enum { MI_TOK = 1, MI_SUB, MI_ACT };
enum {
    ACT_ANGLE_DEG = 1, ACT_ANGLE_RAD, ACT_ANGLE_GRAD,
    ACT_FMT_NORM1, ACT_FMT_NORM2, ACT_FMT_FIX, ACT_FMT_SCI, ACT_FMT_ENG,
    ACT_FRAC, ACT_CPLX, ACT_POLAR,
    ACT_BASE0, ACT_BASE2, ACT_BASE8, ACT_BASE10, ACT_BASE16,
    ACT_ST_EDIT, ACT_ST_RES, ACT_ST_CLEAR, ACT_RESET, ACT_MEM_CLEAR_VARS,
    ACT_BRACKETS
};

typedef struct mitem { const char *label; uint8_t kind, a, b; } mitem_t;

#define TK_(l, t) {l, MI_TOK, t, 0}
#define TKA_(l, t, x) {l, MI_TOK, t, x}
#define SUB_(l, m) {l, MI_SUB, m, 0}
#define ACT_(l, x) {l, MI_ACT, x, 0}

static const mitem_t M_MAIN_[] = {
    SUB_("Hyperbolic", MN_HYP), SUB_("Calculus  d/dx S", MN_CALC), SUB_("Number", MN_NUM),
    SUB_("Probability / Logic", MN_PROB), SUB_("Complex", MN_CPLX), SUB_("Matrix", MN_MATRIX),
    SUB_("Vector", MN_VECTOR), SUB_("Statistics", MN_STAT), SUB_("Distribution", MN_DIST),
    SUB_("Equations", MN_EQN), SUB_("Constants", MN_CONST), SUB_("Unit conversion", MN_CONV),
    SUB_("Memory / Variables", MN_MEM), SUB_("Angle units", MN_ANGLE),
};
static const mitem_t M_HYP_[] = {
    TK_("sinh(", T_SINH), TK_("cosh(", T_COSH), TK_("tanh(", T_TANH),
    TK_("sinh^-1(", T_ASINH), TK_("cosh^-1(", T_ACOSH), TK_("tanh^-1(", T_ATANH),
};
static const mitem_t M_CALC_[] = {
    TK_("\x87( sum f(X),a,b )", T_SIGMA), TK_("d/dx( f(X),x0 )", T_DDX),
    TK_("\x86( f(X),a,b ) integral", T_INTEGRAL), TK_("Solve( f(X)=g ,guess)", T_SOLVE),
    TKA_("X (variable)", T_VAR, VAR_X), TK_("= sign", T_EQ),
};
static const mitem_t M_NUM_[] = {
    TK_("Abs(", T_ABS), TK_("Int(", T_INT), TK_("Intg(", T_INTG), TK_("Rnd(", T_RND),
    TK_("cube root", T_CBRT), TK_("log base b", T_LOGB), TK_("Pol(", T_POL), TK_("Rec(", T_REC),
    TK_("Ran#", T_RAN), TK_("RanInt(", T_RANINT), TK_("DMS(", T_DMSF), TK_("Const(n)", T_CONSTF),
};
static const mitem_t M_PROB_[] = {
    TK_("nPr", T_NPR), TK_("nCr", T_NCR), TK_("x!", T_FACT),
    TK_("and", T_AND), TK_("or", T_OR), TK_("xor", T_XOR), TK_("xnor", T_XNOR),
    TK_("Not(", T_NOT), TK_("Neg(", T_NEGB),
};
static const mitem_t M_CPLX_[] = {
    TK_("i", T_I), TK_("Arg(", T_ARG), TK_("Conjg(", T_CONJ), TK_("Re(", T_RE), TK_("Im(", T_IM),
    TK_("Abs(", T_ABS), TK_("r (radian mark)", T_RADSYM),
};
static const mitem_t M_MATRIX_[] = {
    TKA_("MatA", T_MAT, 0), TKA_("MatB", T_MAT, 1), TKA_("MatC", T_MAT, 2), TKA_("MatD", T_MAT, 3), TKA_("MatAns", T_MAT, 4),
    TK_("[  (matrix start)", T_LBRK), TK_("]", T_RBRK), TK_("; (next row)", T_SEMI), TK_(", (next column)", T_COMMA),
    TK_("Det(", T_DET), TK_("Trn(", T_TRN), TK_("Iden(", T_IDEN), TK_("-> store  (STO)", T_STO),
};
static const mitem_t M_VECTOR_[] = {
    TKA_("VctA", T_VCT, 0), TKA_("VctB", T_VCT, 1), TKA_("VctC", T_VCT, 2), TKA_("VctD", T_VCT, 3), TKA_("VctAns", T_VCT, 4),
    TK_("Vct( a,b,c )", T_VCTLIT), TK_("Dot(", T_DOTP), TK_("Cross(", T_CROSSP), TK_("Angle(", T_ANGLEV), TK_("UnitV(", T_UNITV),
    TK_("Abs( vector )", T_ABS),
};
static const mitem_t M_STAT_[] = {
    ACT_("Edit data", ACT_ST_EDIT), ACT_("Results", ACT_ST_RES), SUB_("Variables", MN_STATVARS),
    SUB_("Regression type", MN_STATTYPE), TK_("y^( estimate y )", T_YHAT), TK_("x^( estimate x )", T_XHAT),
    ACT_("Clear data", ACT_ST_CLEAR),
};
static const mitem_t M_DIST_[] = {
    TK_("NormP( x )  P(t)", T_NORMP), TK_("NormQ( x )  Q(t)", T_NORMQ), TK_("NormR( x )  R(t)", T_NORMR),
};
static const mitem_t M_EQN_[] = {
    TK_("Quad( a,b,c )", T_QUAD), TK_("Cubic( a,b,c,d )", T_CUBIC),
    TK_("Simul2( a1,b1,c1,a2,b2,c2 )", T_SIMUL2), TK_("Simul3( 12 values )", T_SIMUL3), TK_("Solve(", T_SOLVE),
};
static const mitem_t M_MEM_[] = {
    TKA_("A", T_VAR, 0), TKA_("B", T_VAR, 1), TKA_("C", T_VAR, 2), TKA_("D", T_VAR, 3), TKA_("E", T_VAR, 4),
    TKA_("F", T_VAR, 5), TKA_("X", T_VAR, 6), TKA_("Y", T_VAR, 7), TKA_("M", T_VAR, 8),
    TK_("Ans", T_ANS), TK_("PreAns", T_PREANS), ACT_("Clear variables", ACT_MEM_CLEAR_VARS),
};
static const mitem_t M_ANGLE_[] = {
    TK_("\x84  degrees", T_DEGSYM), TK_("r  radians", T_RADSYM), TK_("g  gradians", T_GRADSYM),
    TK_("\x84 ' \"  DMS", T_DMS_D),
};
static const mitem_t M_FORMAT_[] = {
    ACT_("Norm 1", ACT_FMT_NORM1), ACT_("Norm 2", ACT_FMT_NORM2), ACT_("Fix  (decimals)", ACT_FMT_FIX),
    ACT_("Sci  (digits)", ACT_FMT_SCI), ACT_("Eng", ACT_FMT_ENG),
};
static const mitem_t M_BASE_[] = {
    ACT_("Normal (decimal calc)", ACT_BASE0), ACT_("Bin", ACT_BASE2), ACT_("Oct", ACT_BASE8),
    ACT_("Dec", ACT_BASE10), ACT_("Hex", ACT_BASE16),
};
static const mitem_t M_STATTYPE_[] = {
    ACT_("1-Variable", 40), ACT_("y = a+bx", 41), ACT_("y = a+bx+cx^2", 42), ACT_("y = a+b ln x", 43),
    ACT_("y = a e^(bx)", 44), ACT_("y = a b^x", 45), ACT_("y = a x^b", 46), ACT_("y = a+b/x", 47),
};

typedef struct menu_def { const char *title; const mitem_t *items; uint8_t n; } menu_def_t;
#define MD(t, arr) {t, arr, (uint8_t)(sizeof arr / sizeof arr[0])}

static const menu_def_t MENUS[MN_COUNT] = {
    [MN_MAIN] = MD("MENU", M_MAIN_), [MN_HYP] = MD("HYPERBOLIC", M_HYP_), [MN_CALC] = MD("CALCULUS", M_CALC_),
    [MN_NUM] = MD("NUMBER", M_NUM_), [MN_PROB] = MD("PROB / LOGIC", M_PROB_), [MN_CPLX] = MD("COMPLEX", M_CPLX_),
    [MN_MATRIX] = MD("MATRIX", M_MATRIX_), [MN_VECTOR] = MD("VECTOR", M_VECTOR_), [MN_STAT] = MD("STATISTICS", M_STAT_),
    [MN_DIST] = MD("DISTRIBUTION", M_DIST_), [MN_EQN] = MD("EQUATIONS", M_EQN_), [MN_MEM] = MD("MEMORY", M_MEM_),
    [MN_ANGLE] = MD("ANGLE", M_ANGLE_), [MN_FORMAT] = MD("DISPLAY FORMAT", M_FORMAT_), [MN_BASE] = MD("BASE-N", M_BASE_),
    [MN_STATTYPE] = MD("REGRESSION", M_STATTYPE_),
    [MN_CONST] = {"CONSTANTS", 0, 0}, [MN_CONV] = {"CONVERSION", 0, 0}, [MN_STATVARS] = {"STAT VARIABLES", 0, 0},
    [MN_DIGITS] = {"DIGITS", 0, 0}, [MN_MODE] = {"SETUP", 0, 0},
};

static int menu_count(const app_t *a, int id)
{
    (void)a;
    switch (id) {
    case MN_CONST: return const_count();
    case MN_CONV: return conv_count();
    case MN_STATVARS: return SV_COUNT;
    case MN_DIGITS: return 10;
    case MN_MODE: return 8;
    default: return MENUS[id].n;
    }
}

static const char *STATVAR_NAME[SV_COUNT] = {
    "n", "\x87x", "\x87x2", "x-bar", "\x95x (pop)", "sx", "\x87y", "\x87y2", "y-bar", "\x95y (pop)", "sy", "\x87xy",
    "a", "b", "c", "r", "minX", "maxX", "minY", "maxY", "Q1", "Med", "Q3", "\x87x3", "\x87x4", "\x87x2y",
};

static void item_label(const app_t *a, int id, int i, char *buf, size_t cap)
{
    const ctx_t *c = &a->ctx;
    buf[0] = 0;
    switch (id) {
    case MN_CONST: {
        const const_info_t *ci = const_get(i);
        if (ci) snprintf(buf, cap, "%-8s %s", ci->symbol, ci->name);
        return;
    }
    case MN_CONV: {
        const conv_info_t *cv = conv_get(i);
        if (cv) snprintf(buf, cap, "%s \x85 %s", cv->from, cv->to);
        return;
    }
    case MN_STATVARS: snprintf(buf, cap, "%s", STATVAR_NAME[i]); return;
    case MN_DIGITS: snprintf(buf, cap, "%d", i); return;
    case MN_MODE:
        switch (i) {
        case 0: snprintf(buf, cap, "Angle: %s", c->angle == ANG_DEG ? "Degree" : c->angle == ANG_RAD ? "Radian" : "Gradian"); break;
        case 1: snprintf(buf, cap, "Format: %s", c->fmt == FMT_NORM1 ? "Norm 1" : c->fmt == FMT_NORM2 ? "Norm 2" :
                         c->fmt == FMT_FIX ? "Fix" : c->fmt == FMT_SCI ? "Sci" : "Eng"); break;
        case 2: snprintf(buf, cap, "Fraction: %s", c->frac_improper ? "d/c" : "a b/c"); break;
        case 3: snprintf(buf, cap, "Complex: %s", c->cplx ? "On" : "Off"); break;
        case 4: snprintf(buf, cap, "Complex form: %s", c->polar ? "r<theta" : "a+bi"); break;
        case 5: snprintf(buf, cap, "Base-N: %s", c->base == 0 ? "Normal" : c->base == 2 ? "Bin" : c->base == 8 ? "Oct" : c->base == 10 ? "Dec" : "Hex"); break;
        case 6: snprintf(buf, cap, "Statistics type..."); break;
        default: snprintf(buf, cap, "Reset memory..."); break;
        }
        return;
    default: {
        const mitem_t *it = &MENUS[id].items[i];
        const char *mark = "";
        if (id == MN_ANGLE || id == MN_FORMAT || id == MN_BASE || id == MN_STATTYPE) {
            int cur = id == MN_FORMAT ? c->fmt : id == MN_BASE ? (c->base == 0 ? 0 : c->base == 2 ? 1 : c->base == 8 ? 2 : c->base == 10 ? 3 : 4)
                                              : id == MN_STATTYPE ? c->stat.type : -1;
            if (id != MN_ANGLE && cur == i) mark = S_CHECK;
        }
        snprintf(buf, cap, "%s%s", mark, it->label);
        return;
    }
    }
}

void ui_menu_open(app_t *a, int id)
{
    if (a->screen != SCR_MENU) { a->menu_sp = 0; a->menu_prev_screen = a->screen; }
    if (a->menu_sp >= MENU_DEPTH) return;
    a->menu_id[a->menu_sp] = (uint8_t)id;
    a->menu_sel[a->menu_sp] = 0;
    a->menu_sp++;
    a->screen = SCR_MENU;
}

static void menu_close(app_t *a)
{
    if (a->menu_sp > 1) { a->menu_sp--; return; }
    a->menu_sp = 0;
    a->screen = SCR_CALC;
}

static void set_stat_type(app_t *a, int t)
{
    stat_init(&a->ctx.stat, t);   /* new regression type: existing data is cleared */
}

static void select_item(app_t *a)
{
    int sp = a->menu_sp - 1, id = a->menu_id[sp], i = a->menu_sel[sp];
    ctx_t *c = &a->ctx;
    switch (id) {
    case MN_CONST: a->menu_sp = 0; a->screen = SCR_CALC; ui_insert_menu_tok(a, T_CONST, (uint8_t)i); return;
    case MN_CONV: a->menu_sp = 0; a->screen = SCR_CALC; ui_insert_menu_tok(a, T_CONV, (uint8_t)i); return;
    case MN_STATVARS: a->menu_sp = 0; a->screen = SCR_CALC; ui_insert_menu_tok(a, T_STATVAR, (uint8_t)i); return;
    case MN_DIGITS:
        c->fmt_digits = (uint8_t)i;
        a->menu_sp = 0; a->screen = SCR_CALC;
        return;
    case MN_MODE:
        switch (i) {
        case 0: c->angle = (ang_t)((c->angle + 1) % 3); break;
        case 1: ui_menu_open(a, MN_FORMAT); break;
        case 2: c->frac_improper = (uint8_t)!c->frac_improper; break;
        case 3: c->cplx = (uint8_t)!c->cplx; break;
        case 4: c->polar = (uint8_t)!c->polar; break;
        case 5: ui_menu_open(a, MN_BASE); break;
        case 6: ui_menu_open(a, MN_STATTYPE); break;
        default: a->confirm_action = 1; a->menu_sp = 0; a->screen = SCR_CONFIRM; break;
        }
        return;
    default: break;
    }
    {
        const mitem_t *it = &MENUS[id].items[i];
        if (it->kind == MI_SUB) { ui_menu_open(a, it->a); return; }
        if (it->kind == MI_TOK) {
            a->menu_sp = 0; a->screen = SCR_CALC;
            ui_insert_menu_tok(a, it->a, it->b);
            return;
        }
        switch (it->a) {
        case ACT_FMT_NORM1: c->fmt = FMT_NORM1; break;
        case ACT_FMT_NORM2: c->fmt = FMT_NORM2; break;
        case ACT_FMT_ENG: c->fmt = FMT_ENG; break;
        case ACT_FMT_FIX: c->fmt = FMT_FIX; ui_menu_open(a, MN_DIGITS); return;
        case ACT_FMT_SCI: c->fmt = FMT_SCI; ui_menu_open(a, MN_DIGITS); return;
        case ACT_BASE0: c->base = 0; break;
        case ACT_BASE2: c->base = 2; break;
        case ACT_BASE8: c->base = 8; break;
        case ACT_BASE10: c->base = 10; break;
        case ACT_BASE16: c->base = 16; break;
        case ACT_ST_EDIT:
            a->menu_sp = 0; a->screen = SCR_STATEDIT; a->st_row = 0; a->st_col = 0; a->st_editing = 0; a->st_len = 0;
            return;
        case ACT_ST_RES: a->menu_sp = 0; a->screen = SCR_STATRES; a->sr_page = 0; return;
        case ACT_ST_CLEAR: stat_init(&c->stat, c->stat.type); break;
        case ACT_MEM_CLEAR_VARS: {
            int k;
            for (k = 0; k < N_VARS; k++) val_zero(&c->var[k]);
            break;
        }
        default:
            if (it->a >= 40 && it->a <= 47) {
                set_stat_type(a, it->a - 40);
                a->menu_sp = 0; a->screen = SCR_STATEDIT; a->st_row = 0; a->st_col = 0; a->st_editing = 0; a->st_len = 0;
                return;
            }
            break;
        }
        a->menu_sp = 0;
        a->screen = SCR_CALC;
    }
}

void ui_menu_key(app_t *a, int key)
{
    int sp = a->menu_sp - 1, id = a->menu_id[sp], n = menu_count(a, id);
    uint8_t *sel = &a->menu_sel[sp];
    switch (key) {
    case K_UP: *sel = (uint8_t)(*sel == 0 ? n - 1 : *sel - 1); break;
    case K_DOWN: *sel = (uint8_t)(*sel + 1 >= n ? 0 : *sel + 1); break;
    case K_EXE: select_item(a); break;
    case K_AC: case K_MENU: case K_LEFT:
        if (key == K_MENU && a->menu_sp > 0) { a->menu_sp = 0; a->screen = SCR_CALC; }
        else menu_close(a);
        break;
    default:
        {   /* digit keys select the numbered item */
            static const uint8_t dk[10] = {K_0, K_1, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9};
            int d;
            for (d = 0; d < 10; d++) if (dk[d] == key) break;
            if (d == 10) break;
            if (id == MN_DIGITS) { if (d < n) { *sel = (uint8_t)d; select_item(a); } }
            else if (id != MN_CONST && id != MN_CONV && id != MN_STATVARS && d >= 1 && d <= n) { *sel = (uint8_t)(d - 1); select_item(a); }
        }
        break;
    }
}

void ui_menu_render(const app_t *a, lcd_t *l)
{
    int sp = a->menu_sp - 1, id = a->menu_id[sp], n = menu_count(a, id), sel = a->menu_sel[sp];
    int rows = 6, top = sel - rows / 2, i;
    char buf[48];
    if (top > n - rows) top = n - rows;
    if (top < 0) top = 0;
    lcd_fill(l, 0, 0, LCD_W, 8, 1);
    lcd_text(l, 2, 0, MENUS[id].title, 0);
    lcd_invert(l, 2, 0, lcd_text_w(MENUS[id].title, 0), 8);
    {
        char t[24];
        snprintf(t, sizeof t, "%d/%d", sel + 1, n);
        lcd_text_right(l, LCD_W - 2, 1, t, 1);
        lcd_invert(l, LCD_W - 2 - lcd_text_w(t, 1), 1, lcd_text_w(t, 1), 5);
    }
    for (i = 0; i < rows && top + i < n; i++) {
        int y = 10 + i * 9, idx = top + i;
        char num[16];
        item_label(a, id, idx, buf, sizeof buf);
        if ((n <= 9 || id == MN_DIGITS || (id != MN_CONST && id != MN_CONV && id != MN_STATVARS && idx < 9))) snprintf(num, sizeof num, "%d:", id == MN_DIGITS ? idx : idx + 1);
        else snprintf(num, sizeof num, "%2d", idx + 1);
        lcd_text(l, 3, y, num, 0);
        lcd_text(l, 3 + 3 * FONT_W, y, buf, 0);
        if (idx == sel) lcd_invert(l, 0, y - 1, LCD_W - 8, 9);
    }
    if (top > 0) lcd_text(l, LCD_W - 6, 10, S_UP, 0);
    if (top + rows < n) lcd_text(l, LCD_W - 6, 56, S_DOWN, 0);
}
