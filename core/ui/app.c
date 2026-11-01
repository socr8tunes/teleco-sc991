#include "ui/app.h"
#include "parser/tokinfo.h"
#include "parser/lexer.h"
#include <string.h>

#define STO_PENDING 0xFFu

/* ---- key definitions ------------------------------------------------------ */

enum { AK_NONE = 0, AK_TOK, AK_STRUCT, AK_SPECIAL, AK_MENU, AK_VAR, AK_DIGIT };

enum {
    SP_AC = 1, SP_DEL, SP_EXE, SP_LEFT, SP_RIGHT, SP_UP, SP_DOWN, SP_SD, SP_ENG, SP_MPLUS, SP_MMINUS,
    SP_DMSKEY, SP_DMSVIEW, SP_OFF, SP_HOME, SP_END, SP_CONTRAST_UP, SP_CONTRAST_DOWN,
    SP_RESET, SP_STATEDIT, SP_MODE, SP_SHIFT, SP_ALPHA
};

typedef struct act { uint8_t kind, a, b; } act_t;
typedef struct keydef { act_t p, s, al; } keydef_t;

#define NO {AK_NONE, 0, 0}
#define TK(t) {AK_TOK, (t), 0}
#define TKA(t, x) {AK_TOK, (t), (x)}
#define ST(t) {AK_STRUCT, (t), 0}
#define SPC(s) {AK_SPECIAL, (s), 0}
#define MN(m) {AK_MENU, (m), 0}
#define VR(v) {AK_VAR, (v), 0}
#define DG(d) {AK_DIGIT, (d), 0}

static const keydef_t KD[K_COUNT] = {
    [K_SHIFT] = {SPC(SP_SHIFT), NO, NO},
    [K_ALPHA] = {SPC(SP_ALPHA), NO, NO},
    [K_LEFT]  = {SPC(SP_LEFT), SPC(SP_HOME), NO},
    [K_UP]    = {SPC(SP_UP), SPC(SP_CONTRAST_UP), NO},
    [K_DOWN]  = {SPC(SP_DOWN), SPC(SP_CONTRAST_DOWN), NO},
    [K_RIGHT] = {SPC(SP_RIGHT), SPC(SP_END), NO},
    [K_MENU]  = {MN(MN_MAIN), SPC(SP_STATEDIT), NO},
    [K_MODE]  = {SPC(SP_MODE), SPC(SP_RESET), NO},
    [K_FRAC]  = {ST(T_FRAC), ST(T_MFRAC), NO},
    [K_SQRT]  = {ST(T_SQRT), ST(T_ROOT), NO},
    [K_SQ]    = {TK(T_SQ), TK(T_CUBE), NO},
    [K_POW]   = {ST(T_POW), NO, NO},
    [K_LOG]   = {TK(T_LOG), TK(T_EXP10), NO},
    [K_LN]    = {TK(T_LN), TK(T_EXP), NO},
    [K_NEG]   = {TK(T_NEG), NO, VR(VAR_A)},
    [K_DMS]   = {SPC(SP_DMSKEY), SPC(SP_DMSVIEW), VR(VAR_B)},
    [K_HYP]   = {MN(MN_HYP), NO, VR(VAR_C)},
    [K_SIN]   = {TK(T_SIN), TK(T_ASIN), VR(VAR_D)},
    [K_COS]   = {TK(T_COS), TK(T_ACOS), VR(VAR_E)},
    [K_TAN]   = {TK(T_TAN), TK(T_ATAN), VR(VAR_F)},
    [K_STO]   = {TK(T_STO), NO, NO},
    [K_SD]    = {SPC(SP_SD), NO, NO},
    [K_ENG]   = {SPC(SP_ENG), NO, NO},
    [K_MPLUS] = {SPC(SP_MPLUS), SPC(SP_MMINUS), VR(VAR_M)},
    [K_LPAREN] = {TK(T_LP), TK(T_PERCENT), VR(VAR_Y)},
    [K_RPAREN] = {TK(T_RP), TK(T_COMMA), VR(VAR_X)},
    [K_XINV]  = {TK(T_RECIP), NO, NO},
    [K_FACT]  = {TK(T_FACT), NO, NO},
    [K_EE]    = {TK(T_EE), NO, NO},
    [K_7] = {DG(7), MN(MN_CONST), NO}, [K_8] = {DG(8), MN(MN_CONV), NO}, [K_9] = {DG(9), NO, NO},
    [K_DEL]   = {SPC(SP_DEL), NO, NO},
    [K_AC]    = {SPC(SP_AC), SPC(SP_OFF), NO},
    [K_4] = {DG(4), NO, NO}, [K_5] = {DG(5), NO, NO}, [K_6] = {DG(6), NO, NO},
    [K_MUL]   = {TK(T_MUL), TK(T_NPR), NO},
    [K_DIV]   = {TK(T_DIV), TK(T_NCR), NO},
    [K_1] = {DG(1), NO, NO}, [K_2] = {DG(2), NO, NO}, [K_3] = {DG(3), NO, NO},
    [K_ADD]   = {TK(T_ADD), TK(T_POL), NO},
    [K_SUB]   = {TK(T_SUB), TK(T_REC), NO},
    [K_0]     = {DG(0), TK(T_RANINT), NO},
    [K_DOT]   = {TK(T_DOT), TK(T_RAN), NO},
    [K_PI]    = {TK(T_PI), TK(T_E), NO},
    [K_ANS]   = {TK(T_ANS), TK(T_PREANS), NO},
    [K_EXE]   = {SPC(SP_EXE), TK(T_EQ), NO},
};

/* tokens that continue from Ans when typed first after a result */
static int continues_from_ans(const act_t *a)
{
    if (a->kind == AK_STRUCT) return a->a == T_POW;
    if (a->kind != AK_TOK) return 0;
    switch (a->a) {
    case T_ADD: case T_SUB: case T_MUL: case T_DIV: case T_SQ: case T_CUBE: case T_RECIP:
    case T_FACT: case T_PERCENT: case T_NPR: case T_NCR: case T_AND: case T_OR: case T_XOR: case T_XNOR:
    case T_DEGSYM: case T_RADSYM: case T_GRADSYM:
        return 1;
    default:
        return 0;
    }
}

/* ---- history ---------------------------------------------------------------- */

int ui_hist_count(const hist_t *h)
{
    int pos = h->used, n = 0;
    while (pos > 0) {
        int len = h->buf[pos - 1].t;
        pos -= len + 1;
        n++;
    }
    return n;
}

static void hist_push(hist_t *h, const tok_t *t, int n)
{
    if (n <= 0 || n > 250) return;
    /* identical to the newest entry: skip */
    if (h->used > 0 && h->buf[h->used - 1].t == n) {
        int s = h->used - 1 - n, i, same = 1;
        for (i = 0; i < n; i++)
            if (h->buf[s + i].t != t[i].t || h->buf[s + i].a != t[i].a) { same = 0; break; }
        if (same) return;
    }
    if (h->used + n + 1 > HIST_TOKS) {
        /* keep the newest records that leave room for this one */
        int p = h->used, keep_from = h->used;
        while (p > 0) {
            int start = p - (h->buf[p - 1].t + 1);
            if (h->used - start + n + 1 > HIST_TOKS) break;
            keep_from = start;
            p = start;
        }
        memmove(h->buf, &h->buf[keep_from], (size_t)(h->used - keep_from) * sizeof(tok_t));
        h->used = (uint16_t)(h->used - keep_from);
    }
    memcpy(&h->buf[h->used], t, (size_t)n * sizeof(tok_t));
    h->used = (uint16_t)(h->used + n);
    h->buf[h->used++] = mk_tok((uint8_t)n, 0);
}

/* k-th newest record (0 = newest); returns length or -1 */
static int hist_get(const hist_t *h, int k, const tok_t **t)
{
    int pos = h->used;
    for (;;) {
        int len;
        if (pos <= 0) return -1;
        len = h->buf[pos - 1].t;
        if (k == 0) { *t = &h->buf[pos - 1 - len]; return len; }
        pos -= len + 1;
        k--;
    }
}

/* ---- state helpers ----------------------------------------------------------- */

void ui_enter_calc(app_t *a) { a->screen = SCR_CALC; }

void ui_reset_memory(app_t *a)
{
    ctx_init(&a->ctx);
    memset(&a->hist, 0, sizeof a->hist);
    ed_clear(&a->ed);
    a->have_res = 0; a->post_eval = 0; a->err = 0; a->hist_pos = -1;
}

void app_init(app_t *a)
{
    memset(a, 0, sizeof *a);
    ctx_init(&a->ctx);
    a->exact = 1;
    a->hist_pos = -1;
    a->contrast = 32;
    a->screen = SCR_LOGO;
}

void ui_begin_input(app_t *a)
{
    if (a->post_eval) {
        ed_clear(&a->ed);
        a->post_eval = 0;
    }
    a->have_res = 0;
    a->err = 0;
    a->hist_pos = -1;
}

void ui_insert(app_t *a, tok_t k)
{
    if (ed_insert(&a->ed, k)) a->beep = 1;
}

void ui_ac(app_t *a)
{
    ed_clear(&a->ed);
    a->post_eval = 0; a->have_res = 0; a->err = 0; a->hist_pos = -1;
    a->view_idx = 0; a->view_cnt = 0;
}

static void evaluate(app_t *a)
{
    tok_t b[MAX_TOKENS];
    int n, i;
    calc_err_t e;
    if (a->ed.n == 0) return;
    n = ed_build(&a->ed, b, MAX_TOKENS);
    if (n < 0) { a->err = CE_STACK; a->have_res = 0; a->post_eval = 1; return; }
    a->is_pol = a->is_rec = 0;
    for (i = 0; i < n; i++) {
        if (b[i].t == T_POL) a->is_pol = 1;
        if (b[i].t == T_REC) a->is_rec = 1;
    }
    for (i = 0; i < n; i++)
        if (b[i].t == T_STO && b[i].a == STO_PENDING) { a->err = CE_SYNTAX; a->have_res = 0; a->post_eval = 1; return; }
    hist_push(&a->hist, a->ed.t, a->ed.n);
    e = calc_eval_tokens(&a->ctx, b, n, &a->res);
    a->post_eval = 1;
    a->hist_pos = -1;
    a->view_idx = 0;
    a->dms_view = 0;
    a->eng_view = 0;
    if (e == CE_OK) {
        a->have_res = 1;
        a->err = 0;
        a->view_cnt = a->ctx.nres > 1 ? a->ctx.nres : (a->ctx.has_aux ? 2 : 1);
        if (a->res.k == V_MAT || a->res.k == V_VCT) { a->screen = SCR_MATVIEW; a->mv_r = a->mv_c = 0; }
    } else {
        a->have_res = 0;
        a->err = (uint8_t)e;
        a->view_cnt = 1;
    }
}

static void recall_history(app_t *a, int dir)
{
    const tok_t *t;
    int cnt = ui_hist_count(&a->hist), len;
    int pos = a->hist_pos;
    if (cnt == 0) return;
    pos += dir;                       /* dir = +1 older, -1 newer */
    if (pos < 0) { /* past the newest: back to an empty line */
        a->hist_pos = -1;
        ed_clear(&a->ed);
        a->post_eval = 0; a->have_res = 0; a->err = 0;
        return;
    }
    if (pos >= cnt) pos = cnt - 1;
    len = hist_get(&a->hist, pos, &t);
    if (len < 0) return;
    ed_set(&a->ed, t, len);
    a->hist_pos = (int16_t)pos;
    a->post_eval = 0; a->have_res = 0; a->err = 0;
}

static void do_special(app_t *a, int sp)
{
    switch (sp) {
    case SP_AC: ui_ac(a); break;
    case SP_DEL:
        if (a->post_eval) { a->post_eval = 0; a->have_res = 0; a->err = 0; }
        ed_delete(&a->ed);
        break;
    case SP_EXE:
        if (a->post_eval && a->view_idx + 1 < a->view_cnt) { a->view_idx++; break; }
        if (a->post_eval && a->view_cnt > 1) { a->view_idx = 0; break; }
        evaluate(a);
        break;
    case SP_LEFT:
        if (a->post_eval) { a->post_eval = 0; a->have_res = 0; a->err = 0; ed_end(&a->ed); }
        else ed_left(&a->ed);
        break;
    case SP_RIGHT:
        if (a->post_eval) { a->post_eval = 0; a->have_res = 0; a->err = 0; ed_home(&a->ed); }
        else ed_right(&a->ed);
        break;
    case SP_UP:
        if (!a->post_eval && ed_vertical(&a->ed, -1)) break;
        if (a->post_eval && a->view_cnt > 1 && a->view_idx > 0) { a->view_idx--; break; }
        if (a->ed.n == 0 || a->post_eval || a->hist_pos >= 0) recall_history(a, +1);
        break;
    case SP_DOWN:
        if (!a->post_eval && ed_vertical(&a->ed, +1)) break;
        if (a->post_eval && a->view_cnt > 1 && a->view_idx + 1 < a->view_cnt) { a->view_idx++; break; }
        if (a->hist_pos >= 0) recall_history(a, -1);
        break;
    case SP_HOME: ed_home(&a->ed); a->post_eval = 0; break;
    case SP_END: ed_end(&a->ed); a->post_eval = 0; break;
    case SP_SD: if (a->have_res) a->exact = (uint8_t)!a->exact; break;
    case SP_ENG: if (a->have_res) a->eng_view = (uint8_t)!a->eng_view; break;
    case SP_DMSVIEW: if (a->have_res && a->res.k == V_REAL) a->dms_view = (uint8_t)!a->dms_view; break;
    case SP_MPLUS: case SP_MMINUS: {
        /* M = M +/- (displayed result, or the entered expression) */
        val_t v = a->res;
        if (!a->post_eval && a->ed.n > 0) {
            evaluate(a);
            if (!a->have_res) break;
            v = a->res;
        } else if (!a->have_res) break;
        if (v.k == V_REAL) {
            num_t r;
            dec_status e = sp == SP_MPLUS ? num_add(&r, &a->ctx.var[VAR_M].re, &v.re)
                                          : num_sub(&r, &a->ctx.var[VAR_M].re, &v.re);
            if (e == DEC_OK) a->ctx.var[VAR_M].re = r;
            else a->err = CE_MATH;
        }
        break;
    }
    case SP_DMSKEY: {
        /* DMS marks: degrees, minutes, seconds in turn within one number */
        int cnt = 0, i = a->ed.cur - 1;
        ui_begin_input(a);
        while (i >= 0 && (tok_is_digit(a->ed.t[i].t) || a->ed.t[i].t == T_DMS_D || a->ed.t[i].t == T_DMS_M)) {
            if (a->ed.t[i].t == T_DMS_D || a->ed.t[i].t == T_DMS_M) cnt++;
            i--;
        }
        ui_insert(a, mk_tok(cnt == 0 ? T_DMS_D : cnt == 1 ? T_DMS_M : T_DMS_S, 0));
        break;
    }
    case SP_OFF: a->screen = SCR_OFF; break;
    case SP_CONTRAST_UP: if (a->contrast < 63) a->contrast++; break;
    case SP_CONTRAST_DOWN: if (a->contrast > 0) a->contrast--; break;
    case SP_RESET: a->confirm_action = 1; a->screen = SCR_CONFIRM; break;
    case SP_STATEDIT: a->screen = SCR_STATEDIT; a->st_row = 0; a->st_col = 0; a->st_editing = 0; a->st_len = 0; break;
    case SP_MODE: ui_menu_open(a, MN_MODE); break;
    default: break;
    }
}

/* STO leaves a pending target (a = 0xFF); the next variable / matrix / vector key fills it. */
static int fill_store_target(app_t *a, uint8_t tok, uint8_t idx)
{
    editor_t *e = &a->ed;
    uint8_t code;
    if (e->cur == 0 || e->t[e->cur - 1].t != T_STO || e->t[e->cur - 1].a != STO_PENDING) return 0;
    if (tok == T_VAR) code = idx;
    else if (tok == T_MAT && idx < 4) code = (uint8_t)(16 + idx);
    else if (tok == T_VCT && idx < 4) code = (uint8_t)(24 + idx);
    else { a->beep = 1; return 1; }
    e->t[e->cur - 1].a = code;
    return 1;
}

static void do_action(app_t *a, const act_t *x)
{
    switch (x->kind) {
    case AK_TOK:
        if ((x->a == T_VAR || x->a == T_MAT || x->a == T_VCT) && !a->post_eval && fill_store_target(a, x->a, x->b)) break;
        if (x->a == T_STO) {
            if (a->post_eval) { ed_clear(&a->ed); a->post_eval = 0; a->have_res = 0; a->err = 0; ui_insert(a, mk_tok(T_ANS, 0)); }
            ui_insert(a, mk_tok(T_STO, STO_PENDING));
            break;
        }
        if (a->post_eval && continues_from_ans(x)) {
            ed_clear(&a->ed);
            a->post_eval = 0; a->have_res = 0; a->err = 0;
            ui_insert(a, mk_tok(T_ANS, 0));
        } else {
            ui_begin_input(a);
        }
        ui_insert(a, mk_tok(x->a, x->b));
        break;
    case AK_STRUCT:
        if (a->post_eval && continues_from_ans(x)) {
            ed_clear(&a->ed);
            a->post_eval = 0; a->have_res = 0; a->err = 0;
            ui_insert(a, mk_tok(T_ANS, 0));
        } else {
            ui_begin_input(a);
        }
        if (ed_insert_struct(&a->ed, x->a)) a->beep = 1;
        break;
    case AK_DIGIT:
        ui_begin_input(a);
        if (a->ctx.base && x->a >= a->ctx.base && a->ctx.base != 16) { a->beep = 1; break; }
        ui_insert(a, mk_tok((uint8_t)(T_D0 + x->a), 0));
        break;
    case AK_VAR:
        if (!a->post_eval && fill_store_target(a, T_VAR, x->a)) break;
        ui_begin_input(a);
        if (a->ctx.base == 16 && x->a <= VAR_F) ui_insert(a, mk_tok((uint8_t)(T_DA + x->a), 0));
        else ui_insert(a, mk_tok(T_VAR, x->a));
        break;
    case AK_MENU:
        ui_menu_open(a, x->a);
        break;
    case AK_SPECIAL:
        do_special(a, x->a);
        break;
    default:
        break;
    }
}

int ui_set_expr_text(app_t *a, const char *text)
{
    tok_t t[MAX_TOKENS];
    int n = 0;
    if (lex_text(text, a->ctx.base, t, MAX_TOKENS, &n) != CE_OK || n > ED_CAP) return -1;
    ed_set(&a->ed, t, n);
    a->post_eval = 0; a->have_res = 0; a->err = 0; a->hist_pos = -1;
    return 0;
}

void ui_insert_menu_tok(app_t *a, uint8_t t, uint8_t aux)
{
    act_t x;
    x.kind = AK_TOK; x.a = t; x.b = aux;
    do_action(a, &x);
}

void app_key(app_t *a, int key)
{
    const keydef_t *kd;
    const act_t *x;
    if (key < 0 || key >= K_COUNT) return;
    a->last_key_ms = a->now_ms;
    a->beep = 0;
    if (a->screen == SCR_OFF) {
        if (key == K_AC) { a->screen = SCR_CALC; a->shift = a->alpha = 0; }
        return;
    }
    if (a->screen == SCR_LOGO) { a->screen = SCR_CALC; return; }

    if (key == K_SHIFT) { a->shift = (uint8_t)!a->shift; a->alpha = 0; return; }
    if (key == K_ALPHA) { a->alpha = (uint8_t)!a->alpha; a->shift = 0; return; }

    switch (a->screen) {
    case SCR_MENU: ui_menu_key(a, key); a->shift = a->alpha = 0; return;
    case SCR_STATEDIT: ui_stat_key(a, key); a->shift = a->alpha = 0; return;
    case SCR_STATRES: ui_statres_key(a, key); a->shift = a->alpha = 0; return;
    case SCR_MATVIEW: ui_matview_key(a, key); a->shift = a->alpha = 0; return;
    case SCR_CONFIRM: ui_confirm_key(a, key); a->shift = a->alpha = 0; return;
    default: break;
    }

    kd = &KD[key];
    x = a->shift ? &kd->s : a->alpha ? &kd->al : &kd->p;
    if (x->kind == AK_NONE) { a->shift = a->alpha = 0; a->beep = 1; return; }   /* no such function */
    a->shift = a->alpha = 0;
    do_action(a, x);
}

void app_tick(app_t *a, uint32_t now_ms)
{
    a->now_ms = now_ms;
    a->blink = (uint8_t)((now_ms / 500u) & 1u);
    if (a->screen == SCR_LOGO && now_ms - a->logo_ms >= LOGO_MS) a->screen = SCR_CALC;
    if (a->screen != SCR_OFF && a->screen != SCR_LOGO && now_ms - a->last_key_ms >= AUTO_OFF_MS)
        a->screen = SCR_OFF;
}
