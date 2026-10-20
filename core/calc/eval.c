/*
 * Stack VM that executes compiled programs.
 */
#include <string.h>
#include "calc/eval.h"
#include "parser/lexer.h"
#include "calc/nummethods.h"
#include "values/cplx.h"

typedef struct ev {
    ctx_t        *c;
    const prog_t *p;
    val_t         st[EVAL_STACK];
    int           sp;
    int           depth;
    calc_err_t    err;
} ev_t;

static calc_err_t map_status(dec_status s)
{
    switch (s) {
    case DEC_OK: case DEC_UNDERFLOW: return CE_OK;
    case DEC_PARSE: return CE_SYNTAX;
    default: return CE_MATH;
    }
}

static void fail(ev_t *e, calc_err_t er) { if (!e->err) e->err = er; }
static void fail_st(ev_t *e, dec_status s) { calc_err_t er = map_status(s); if (er) fail(e, er); }

static val_t *push(ev_t *e)
{
    if (e->sp >= EVAL_STACK) { fail(e, CE_STACK); return &e->st[EVAL_STACK - 1]; }
    return &e->st[e->sp++];
}

static int is_real(const val_t *v) { return v->k == V_REAL; }

static void set_real(val_t *v, const num_t *n) { val_real(v, n); }


/* ---- complex support ---------------------------------------------------- */

static int is_num(const val_t *v) { return v->k == V_REAL || v->k == V_CPLX; }

static void to_cx(const val_t *v, cx_t *c)
{
    if (v->k == V_CPLX) { c->re = v->re; c->im = v->im; }
    else cx_from_real(c, &v->re);
}

/* store a complex result; demote to real when the imaginary part is zero */
static void from_cx(val_t *v, const cx_t *c)
{
    memset(v, 0, sizeof *v);
    if (cx_is_real(c)) {
        v->k = V_REAL;
        v->re = c->re;
        num_zero(&v->im);
    } else {
        v->k = V_CPLX;
        v->re = c->re;
        v->im = c->im;
    }
}

static void cx_binary(ev_t *e, int op, const val_t *a, const val_t *b, val_t *r)
{
    cx_t x, y, z;
    dec_status s = DEC_OK;
    to_cx(a, &x);
    to_cx(b, &y);
    switch (op) {
    case T_ADD: s = cx_add(&z, &x, &y); break;
    case T_SUB: case T_EQ: s = cx_sub(&z, &x, &y); break;
    case T_MUL: s = cx_mul(&z, &x, &y); break;
    case T_DIV: s = cx_div(&z, &x, &y); break;
    case T_POW: {
        dec_t ed;
        fn_e(&ed);
        /* the constant e raised to a complex power is exp(), not exp(w ln e) */
        if (a->k == V_REAL && dec_cmp(&a->re.v, &ed) == 0) s = cx_exp(&z, &y);
        else s = cx_pow(&z, &x, &y);
        break;
    }
    default: fail(e, CE_ARG); return;
    }
    if (DEC_STATUS_IS_ERROR(s)) { fail_st(e, s); return; }
    from_cx(r, &z);
}

/* function of a complex argument. Returns 0 when handled. */
static int cx_function(ev_t *e, int fn, const val_t *a, val_t *r)
{
    cx_t x, z;
    num_t n;
    dec_status s = DEC_OK;
    to_cx(a, &x);
    switch (fn) {
    case T_SQRT: s = cx_sqrt(&z, &x); break;
    case T_EXP: s = cx_exp(&z, &x); break;
    case T_LN: s = cx_ln(&z, &x); break;
    case T_LOG: s = cx_log10(&z, &x); break;
    case T_SIN: s = cx_sin(&z, &x); break;
    case T_COS: s = cx_cos(&z, &x); break;
    case T_TAN: s = cx_tan(&z, &x); break;
    case T_SINH: s = cx_sinh(&z, &x); break;
    case T_COSH: s = cx_cosh(&z, &x); break;
    case T_TANH: s = cx_tanh(&z, &x); break;
    case T_CONJ: s = cx_conj(&z, &x); break;
    case T_ABS: s = cx_abs(&n, &x); if (!s) set_real(r, &n); fail_st(e, s); return 0;
    case T_ARG: s = cx_arg(&n, &x, e->c->angle); if (!s) set_real(r, &n); fail_st(e, s); return 0;
    case T_RE: set_real(r, &x.re); return 0;
    case T_IM: set_real(r, &x.im); return 0;
    default: return -1;
    }
    if (DEC_STATUS_IS_ERROR(s)) { fail_st(e, s); return 0; }
    from_cx(r, &z);
    return 0;
}

/* ---- real operations --------------------------------------------------- */

static void real_binary(ev_t *e, int op, const num_t *a, const num_t *b, num_t *r)
{
    dec_status s = DEC_OK;
    switch (op) {
    case T_ADD: s = num_add(r, a, b); break;
    case T_SUB: case T_EQ: s = num_sub(r, a, b); break;
    case T_MUL: s = num_mul(r, a, b); break;
    case T_DIV: s = num_div(r, a, b); break;
    case T_POW: s = num_pow(r, a, b); break;
    case T_NPR: s = num_npr(r, a, b); break;
    case T_NCR: s = num_ncr(r, a, b); break;
    default: fail(e, CE_SYNTAX); return;
    }
    fail_st(e, s);
}

static void real_fn1(ev_t *e, int fn, const num_t *a, num_t *r)
{
    ang_t m = e->c->angle;
    dec_status s = DEC_OK;
    dec_t d;
    switch (fn) {
    case T_SIN: s = num_sin(r, a, m); break;
    case T_COS: s = num_cos(r, a, m); break;
    case T_TAN: s = num_tan(r, a, m); break;
    case T_ASIN: s = num_asin(r, a, m); break;
    case T_ACOS: s = num_acos(r, a, m); break;
    case T_ATAN: s = num_atan(r, a, m); break;
    case T_SINH: s = fn_sinh(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_COSH: s = fn_cosh(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_TANH: s = fn_tanh(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_ASINH: s = fn_asinh(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_ACOSH: s = fn_acosh(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_ATANH: s = fn_atanh(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_LOG: s = fn_log10(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_LN: s = fn_ln(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_EXP: s = fn_exp(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_ABS: s = num_abs(r, a); break;
    case T_SQRT: s = num_sqrt(r, a); break;
    case T_CBRT: {
        num_t three;
        num_from_int(&three, 3);
        s = num_root(r, &three, a);
        break;
    }
    case T_INT: s = fn_trunc(&d, &a->v); s = num_wrap(r, &d, s); break;
    case T_INTG: s = fn_floor(&d, &a->v); s = num_wrap(r, &d, s); break;
    default: fail(e, CE_SYNTAX); return;
    }
    fail_st(e, s);
}

static void real_post(ev_t *e, int op, const num_t *a, num_t *r)
{
    dec_status s = DEC_OK;
    num_t k;
    switch (op) {
    case T_FACT: s = num_fact(r, a); break;
    case T_SQ: s = num_mul(r, a, a); break;
    case T_CUBE: { num_t t; s = num_mul(&t, a, a); if (!s) s = num_mul(r, &t, a); break; }
    case T_RECIP: num_from_int(&k, 1); s = num_div(r, &k, a); break;
    case T_PERCENT: num_from_int(&k, 100); s = num_div(r, a, &k); break;
    case T_DEGSYM: s = num_convert_angle(r, a, ANG_DEG, e->c->angle); break;
    case T_RADSYM: s = num_convert_angle(r, a, ANG_RAD, e->c->angle); break;
    case T_GRADSYM: s = num_convert_angle(r, a, ANG_GRAD, e->c->angle); break;
    default: fail(e, CE_SYNTAX); return;
    }
    fail_st(e, s);
}

static uint32_t rng_next(ctx_t *c)
{
    uint32_t x = c->rng;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    c->rng = x;
    return x;
}

static void run(ev_t *e, int pc, int end);

/* ---- block functions (sigma) ------------------------------------------ */

typedef struct bodyctx {
    ev_t *e;
    int   pc, len;
} bodyctx_t;

/* y = body(X = x) as a numerical-methods callback. */
static calc_err_t body_cb(void *user, const dec_t *x, dec_t *y)
{
    bodyctx_t *b = (bodyctx_t *)user;
    ev_t *e = b->e;
    int sp = e->sp;
    calc_err_t er;
    num_t t;
    num_from_dec(&t, x);
    val_real(&e->c->var[VAR_X], &t);
    run(e, b->pc, b->pc + b->len);
    er = e->err;
    if (!er && (e->sp != sp + 1 || !is_real(&e->st[sp]))) er = CE_ARG;
    if (!er) *y = e->st[sp].re.v;
    e->sp = sp;
    if (er == CE_MATH || er == CE_ARG) e->err = CE_OK;   /* undefined at this point: caller decides */
    return er;
}

static void block_fn(ev_t *e, const ins_t *in, int body_pc)
{
    int argc = in->a, blen = in->y;
    val_t save_x = e->c->var[VAR_X];
    bodyctx_t b;

    b.e = e; b.pc = body_pc; b.len = blen;

    if (in->x == T_SIGMA && argc == 3) {
        val_t *vb = &e->st[e->sp - 1], *va = &e->st[e->sp - 2];
        int64_t lo, hi, i;
        num_t acc, term;
        if (!is_real(va) || !is_real(vb) || dec_to_i64(&va->re.v, &lo) || dec_to_i64(&vb->re.v, &hi)) {
            fail(e, CE_ARG);
            return;
        }
        e->sp -= 2;
        if (hi - lo > 100000) { fail(e, CE_TIME); return; }
        num_from_int(&acc, 0);
        for (i = lo; i <= hi && !e->err; i++) {
            val_int(&e->c->var[VAR_X], i);
            run(e, body_pc, body_pc + blen);
            if (e->err) break;
            term = e->st[--e->sp].re;
            if (!is_real(&e->st[e->sp])) { fail(e, CE_ARG); break; }
            fail_st(e, num_add(&acc, &acc, &term));
        }
        e->c->var[VAR_X] = save_x;
        if (!e->err) set_real(push(e), &acc);
        return;
    }
    if ((in->x == T_DDX && argc == 2) || (in->x == T_SOLVE && argc == 2)) {
        val_t *vx = &e->st[e->sp - 1];
        dec_t out, resid;
        calc_err_t er;
        if (!is_real(vx)) { fail(e, CE_ARG); return; }
        e->sp--;
        if (in->x == T_DDX) {
            er = nm_derivative(body_cb, &b, &vx->re.v, &out);
        } else {
            er = nm_solve(body_cb, &b, &vx->re.v, &out, &resid);
        }
        e->c->var[VAR_X] = save_x;
        if (er) { fail(e, er); return; }
        {
            num_t t;
            num_from_dec(&t, &out);
            if (in->x == T_SOLVE) {
                val_real(&e->c->var[VAR_X], &t);    /* SOLVE leaves the root in X */
                e->c->has_aux = 1;
                num_from_dec(&e->c->aux, &resid);
            }
            set_real(push(e), &t);
        }
        return;
    }
    if (in->x == T_INTEGRAL && argc == 3) {
        val_t *vb = &e->st[e->sp - 1], *va = &e->st[e->sp - 2];
        dec_t out;
        calc_err_t er;
        if (!is_real(va) || !is_real(vb)) { fail(e, CE_ARG); return; }
        e->sp -= 2;
        er = nm_integrate(body_cb, &b, &va->re.v, &vb->re.v, &out);
        e->c->var[VAR_X] = save_x;
        if (er) { fail(e, er); return; }
        { num_t t; num_from_dec(&t, &out); set_real(push(e), &t); }
        return;
    }
    e->c->var[VAR_X] = save_x;
    fail(e, CE_ARG);
}

static void run(ev_t *e, int pc, int end)
{
    const prog_t *p = e->p;
    if (++e->depth > MAX_DEPTH) { fail(e, CE_STACK); e->depth--; return; }
    while (pc < end && !e->err) {
        const ins_t *in = &p->code[pc++];
        val_t *a, *b;
        if (e->c->budget == 0) { fail(e, CE_TIME); break; }
        e->c->budget--;
        switch (in->op) {
        case OP_CONST: set_real(push(e), &p->consts[in->x]); break;
        case OP_POP: if (e->sp > 0) e->sp--; break;
        case OP_UNEG:
            a = &e->st[e->sp - 1];
            if (a->k == V_CPLX) { cx_t x; to_cx(a, &x); fail_st(e, cx_neg(&x, &x)); if (!e->err) from_cx(a, &x); break; }
            if (!is_real(a)) { fail(e, CE_ARG); break; }
            fail_st(e, num_neg(&a->re, &a->re));
            break;
        case T_PI: { num_t t; num_pi(&t); set_real(push(e), &t); break; }
        case T_E: {
            dec_t d; num_t t;
            fn_e(&d); num_from_dec(&t, &d); set_real(push(e), &t);
            break;
        }
        case T_I:
            if (!e->c->cplx) { fail(e, CE_SYNTAX); break; }
            { val_t *v = push(e); num_t z, o; memset(v, 0, sizeof *v); num_from_int(&z, 0); num_from_int(&o, 1); v->k = V_CPLX; v->re = z; v->im = o; }
            break;
        case T_ANS: *push(e) = e->c->ans; break;
        case T_PREANS: *push(e) = e->c->preans; break;
        case T_VAR: *push(e) = e->c->var[in->a]; break;
        case T_RAN: {
            num_t t; dec_t d;
            int64_t k = (int64_t)(rng_next(e->c) % 1000u);
            dec_status s = dec_from_i64(&d, k);
            if (!s) { dec_t th; dec_from_i64(&th, 1000); s = dec_div(&d, &d, &th); }
            num_from_dec(&t, &d);
            fail_st(e, s);
            set_real(push(e), &t);
            break;
        }
        case T_STO:
            if (e->sp < 1) { fail(e, CE_SYNTAX); break; }
            e->c->var[in->a] = e->st[e->sp - 1];
            break;
        case T_ADD: case T_SUB: case T_EQ: case T_MUL: case T_DIV: case T_POW: case T_NPR: case T_NCR:
            if (e->sp < 2) { fail(e, CE_SYNTAX); break; }
            b = &e->st[--e->sp];
            a = &e->st[e->sp - 1];
            if (is_real(a) && is_real(b)) {
                int want_cx = 0;
                if (e->c->cplx && in->op == T_POW && a->re.v.neg && !dec_is_zero(&a->re.v)) {
                    int64_t iv;
                    want_cx = dec_to_i64(&b->re.v, &iv) != 0;   /* negative base, non-integer exponent */
                }
                if (!want_cx) {
                    num_t t;
                    real_binary(e, in->op, &a->re, &b->re, &t);
                    if (!e->err) set_real(a, &t);
                    break;
                }
            }
            if (!is_num(a) || !is_num(b) || in->op == T_NPR || in->op == T_NCR || (!e->c->cplx && (a->k == V_CPLX || b->k == V_CPLX) && 0)) { fail(e, CE_ARG); break; }
            { val_t t; cx_binary(e, in->op, a, b, &t); if (!e->err) *a = t; }
            break;
        case T_FACT: case T_SQ: case T_CUBE: case T_RECIP: case T_PERCENT:
        case T_DEGSYM: case T_RADSYM: case T_GRADSYM:
            a = &e->st[e->sp - 1];
            if (a->k == V_CPLX && (in->op == T_SQ || in->op == T_CUBE || in->op == T_RECIP || in->op == T_PERCENT)) {
                val_t t, k;
                if (in->op == T_SQ) { cx_binary(e, T_MUL, a, a, &t); }
                else if (in->op == T_CUBE) { val_t sq; cx_binary(e, T_MUL, a, a, &sq); cx_binary(e, T_MUL, &sq, a, &t); }
                else { val_int(&k, in->op == T_RECIP ? 1 : 100); if (in->op == T_RECIP) cx_binary(e, T_DIV, &k, a, &t); else cx_binary(e, T_DIV, a, &k, &t); }
                if (!e->err) *a = t;
                break;
            }
            if (!is_real(a)) { fail(e, CE_ARG); break; }
            { num_t t; real_post(e, in->op, &a->re, &t); if (!e->err) set_real(a, &t); }
            break;
        case OP_DMS: {
            int k = in->a, i;
            num_t parts[3], t, sixty, v;
            if (e->sp < k) { fail(e, CE_SYNTAX); break; }
            for (i = 0; i < k; i++) {
                a = &e->st[e->sp - k + i];
                if (!is_real(a)) { fail(e, CE_ARG); break; }
                parts[i] = a->re;
            }
            if (e->err) break;
            e->sp -= k;
            v = parts[0];
            num_from_int(&sixty, 60);
            if (k >= 2) { num_div(&t, &parts[1], &sixty); fail_st(e, num_add(&v, &v, &t)); }
            if (k >= 3) {
                num_t s3600;
                num_from_int(&s3600, 3600);
                num_div(&t, &parts[2], &s3600);
                fail_st(e, num_add(&v, &v, &t));
            }
            set_real(push(e), &v);
            break;
        }
        case OP_FN: {
            int fn = in->x, argc = in->a;
            if (e->sp < argc) { fail(e, CE_SYNTAX); break; }
            if (argc == 1) {
                a = &e->st[e->sp - 1];
                if (a->k == V_CPLX) {
                    val_t t;
                    if (cx_function(e, fn, a, &t) != 0) { fail(e, CE_ARG); break; }
                    if (!e->err) *a = t;
                    break;
                }
                if (!is_real(a)) { fail(e, CE_ARG); break; }
                if (fn == T_ARG || fn == T_CONJ || fn == T_RE || fn == T_IM) {
                    val_t t;
                    if (!e->c->cplx) { fail(e, CE_SYNTAX); break; }
                    cx_function(e, fn, a, &t);
                    if (!e->err) *a = t;
                    break;
                }
                {
                    num_t t;
                    int save = e->err;
                    real_fn1(e, fn, &a->re, &t);
                    if (e->err == CE_MATH && e->c->cplx && (fn == T_SQRT || fn == T_LN || fn == T_LOG)) {
                        val_t c;
                        e->err = save;
                        if (cx_function(e, fn, a, &c) == 0 && !e->err) { *a = c; break; }
                    }
                    if (!e->err) set_real(a, &t);
                }
            } else if (argc == 2 && (fn == T_LOG || fn == T_ROOT)) {
                num_t t; dec_status s;
                a = &e->st[e->sp - 2];
                b = &e->st[e->sp - 1];
                if (!is_real(a) || !is_real(b)) { fail(e, CE_ARG); break; }
                if (fn == T_LOG) {
                    dec_t d;
                    s = fn_log(&d, &a->re.v, &b->re.v);
                    s = num_wrap(&t, &d, s);
                } else {
                    s = num_root(&t, &a->re, &b->re);
                }
                fail_st(e, s);
                e->sp--;
                if (!e->err) set_real(&e->st[e->sp - 1], &t);
            } else {
                fail(e, CE_ARG);
            }
            break;
        }
        case OP_BLOCK_FN:
            block_fn(e, in, pc);
            pc += in->y;
            break;
        default:
            fail(e, CE_SYNTAX);
            break;
        }
    }
    e->depth--;
}

calc_err_t eval_prog(ctx_t *c, const prog_t *p, val_t *out)
{
    ev_t *e;
    static ev_t storage;      /* keeps the large stack off the (small) MCU call stack */
    e = &storage;
    memset(e, 0, sizeof *e);
    e->c = c; e->p = p;
    c->budget = VM_BUDGET;
    c->has_aux = 0;
    if (p->has_eq) return CE_SYNTAX;
    run(e, 0, p->n);
    if (e->err) return e->err;
    if (e->sp != 1) return CE_SYNTAX;
    *out = e->st[0];
    return CE_OK;
}

calc_err_t calc_eval_tokens(ctx_t *c, const tok_t *t, int n, val_t *out)
{
    static prog_t pr;
    calc_err_t er = compile_tokens(t, n, &pr);
    if (er) return er;
    er = eval_prog(c, &pr, out);
    if (!er) { c->preans = c->ans; c->ans = *out; }
    return er;
}

calc_err_t calc_eval_text(ctx_t *c, const char *text, val_t *out)
{
    tok_t toks[MAX_TOKENS];
    int n = 0;
    calc_err_t er = lex_text(text, toks, MAX_TOKENS, &n);
    if (er) return er;
    return calc_eval_tokens(c, toks, n, out);
}
