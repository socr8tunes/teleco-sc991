/*
 * Recursive-descent compiler: tokens -> RPN program.
 *
 * Precedence, loosest to tightest (matches mainstream scientific calculators):
 *   or xor xnor  <  and  <  + -  <  * /  <  implicit multiplication  <  nPr nCr
 *   <  unary minus  <  ^ (right associative)  <  postfix (! x² % deg ...)
 * Hence  -2^2 = -4,  1/2pi = 1/(2pi),  2^-3 = 1/8,  2^3^2 = 512.
 */
#include <string.h>
#include "parser/program.h"
#include "parser/lexer.h"

#define MAT_MAX_R 4

typedef struct cs {
    const tok_t *t;
    int n, p;
    prog_t *pr;
    int base;
    int depth;       /* recursion guard */
    int nest;        /* user-visible nesting: brackets, function calls, structures */
    calc_err_t err;
} cs_t;

static int peek(const cs_t *c) { return c->p < c->n ? c->t[c->p].t : T_NONE; }
static int accept(cs_t *c, int t)
{
    if (peek(c) == t) { c->p++; return 1; }
    return 0;
}

static void fail(cs_t *c, calc_err_t e) { if (!c->err) c->err = e; }

static void emit(cs_t *c, uint8_t op, uint8_t a, uint16_t x, uint16_t y)
{
    ins_t *i;
    if (c->err) return;
    if (c->pr->n >= MAX_CODE) { fail(c, CE_STACK); return; }
    i = &c->pr->code[c->pr->n++];
    i->op = op; i->a = a; i->x = x; i->y = y;
}

static void emit_const(cs_t *c, const num_t *v)
{
    if (c->err) return;
    if (c->pr->nconst >= MAX_CONSTS) { fail(c, CE_STACK); return; }
    c->pr->consts[c->pr->nconst].d = v->v;
    c->pr->consts[c->pr->nconst].is_int = (uint8_t)(ex_valid(&v->ex) != 0);
    emit(c, OP_CONST, 0, (uint16_t)c->pr->nconst, 0);
    c->pr->nconst++;
}

static int is_digit_tok(int t) { return t >= T_D0 && t <= T_DF; }

static int is_fn_tok(int t)
{
    return (t >= T_SIN && t <= T_NEGB);
}

static int starts_atom(int t)
{
    if (is_digit_tok(t) || t == T_DOT || t == T_BASEPFX) return 1;
    if (is_fn_tok(t)) return 1;
    switch (t) {
    case T_LP: case T_LBRK: case T_FRAC: case T_MFRAC: case T_SQRT: case T_ROOT: case T_LOGB:
    case T_PI: case T_E: case T_I: case T_ANS: case T_PREANS: case T_RAN:
    case T_CONST: case T_STATVAR: case T_VAR: case T_MAT: case T_VCT:
        return 1;
    default:
        return 0;
    }
}

static void parse_or(cs_t *c);

/* arity table for function tokens: min, max */
static void arity(int t, int *lo, int *hi)
{
    *lo = *hi = 1;
    switch (t) {
    case T_LOG: *lo = 1; *hi = 2; break;
    case T_CONVF:
    case T_GCD: case T_LCM: case T_QUOT: case T_REM: case T_POIPD: case T_POICD:
    case T_RANINT: case T_POL: case T_REC: case T_DOTP: case T_CROSSP: case T_ANGLEV:
    case T_DDX: case T_SOLVE:
        *lo = *hi = 2; break;
    case T_SIMUL4: *lo = *hi = 20; break;
    case T_SIGMA: case T_PROD: case T_INTEGRAL: case T_BINPD: case T_BINCD: *lo = *hi = 3; break;
    case T_INVN: *lo = 1; *hi = 3; break;
    case T_VCTLIT: *lo = 2; *hi = 3; break;
    case T_QUAD: *lo = *hi = 3; break;
    case T_CUBIC: *lo = *hi = 4; break;
    case T_SIMUL2: *lo = *hi = 6; break;
    case T_SIMUL3: *lo = *hi = 12; break;
    default: break;
    }
}

/* integer literal in base-n mode (32-bit two's complement) */
static void parse_base_number(cs_t *c)
{
    int base = c->base;
    uint64_t v = 0;
    int digits = 0;
    num_t n;
    int64_t iv;

    if (peek(c) == T_BASEPFX) { base = c->t[c->p].a; c->p++; }
    while (is_digit_tok(peek(c))) {
        int d = peek(c) - T_D0;
        if (d >= base) { fail(c, CE_SYNTAX); return; }
        v = v * (uint64_t)base + (uint64_t)d;
        if (v > 0xFFFFFFFFu) { fail(c, CE_MATH); return; }
        digits++;
        c->p++;
    }
    if (!digits) { fail(c, CE_SYNTAX); return; }
    iv = (int64_t)v;
    if (base != 10 && iv >= 0x80000000LL) iv -= 0x100000000LL;     /* two's complement pattern */
    else if (base == 10 && iv > 2147483647LL) { fail(c, CE_MATH); return; }
    num_from_int(&n, iv);
    emit_const(c, &n);
}

static void parse_number(cs_t *c)
{
    if (c->base) { parse_base_number(c); return; }
    char buf[48];
    int n = 0, dots = 0, hasexp = 0, digits = 0;
    num_t v;
    dec_t d;
    dec_status st;

    while (!c->err) {
        int t = peek(c);
        if (is_digit_tok(t)) { if (n < 40) buf[n++] = (char)('0' + (t - T_D0)); else { fail(c, CE_SYNTAX); return; } digits++; c->p++; }
        else if (t == T_DOT) { if (dots++) { fail(c, CE_SYNTAX); return; } buf[n++] = '.'; c->p++; }
        else break;
    }
    if (digits == 0) { fail(c, CE_SYNTAX); return; }
    if (peek(c) == T_EE) {
        int ed = 0;
        c->p++;
        hasexp = 1;
        buf[n++] = 'E';
        if (accept(c, T_NEG)) buf[n++] = '-';
        while (is_digit_tok(peek(c))) {
            if (ed < 4) buf[n++] = (char)('0' + (peek(c) - T_D0)); else { fail(c, CE_SYNTAX); return; }
            ed++; c->p++;
        }
        if (!ed) { fail(c, CE_SYNTAX); return; }
    }
    buf[n] = 0;
    st = dec_from_str(&d, buf);
    if (st != DEC_OK && st != DEC_UNDERFLOW) { fail(c, st == DEC_OVERFLOW ? CE_MATH : CE_SYNTAX); return; }
    if (!dots && !hasexp && digits <= 15) {
        int64_t iv = 0;
        int i;
        for (i = 0; i < n; i++) iv = iv * 10 + (buf[i] - '0');
        num_from_int(&v, iv);
    } else {
        num_from_dec(&v, &d);
    }
    emit_const(c, &v);
}

/* number followed by DMS marks: d°m's" */
static void parse_number_dms(cs_t *c)
{
    int parts = 1;
    parse_number(c);
    if (c->err || peek(c) != T_DMS_D) return;
    c->p++;
    if (is_digit_tok(peek(c)) || peek(c) == T_DOT) {
        parse_number(c);
        parts = 2;
        if (!accept(c, T_DMS_M)) { fail(c, CE_SYNTAX); return; }
        if (is_digit_tok(peek(c)) || peek(c) == T_DOT) {
            parse_number(c);
            parts = 3;
            if (!accept(c, T_DMS_S)) { fail(c, CE_SYNTAX); return; }
        }
    }
    emit(c, OP_DMS, (uint8_t)parts, 0, 0);
}

static void rev_ins(ins_t *a, int n)
{
    int i;
    for (i = 0; i < n / 2; i++) { ins_t t = a[i]; a[i] = a[n - 1 - i]; a[n - 1 - i] = t; }
}

/* move the first blen instructions of [p0, end) behind the rest, in place (three reversals):
 * no temporary buffer, so nested function calls do not pay for one on the call stack */
static void rotate_body(cs_t *c, int p0, int blen)
{
    prog_t *pr = c->pr;
    int total = pr->n - p0;
    rev_ins(&pr->code[p0], blen);
    rev_ins(&pr->code[p0 + blen], total - blen);
    rev_ins(&pr->code[p0], total);
}

static void parse_fn(cs_t *c, int ft)
{
    int lo, hi, argc = 0, body = (ft == T_SIGMA || ft == T_PROD || ft == T_DDX || ft == T_INTEGRAL || ft == T_SOLVE);
    int p0 = c->pr->n, blen = 0;

    arity(ft, &lo, &hi);
    for (;;) {
        parse_or(c);
        if (ft == T_SOLVE && argc == 0 && accept(c, T_EQ)) {
            parse_or(c);
            emit(c, T_EQ, 0, 0, 0);
        }
        argc++;
        if (body && argc == 1) blen = c->pr->n - p0;
        if (c->err) return;
        if (accept(c, T_COMMA)) continue;
        break;
    }
    if (!accept(c, T_RP)) { fail(c, CE_SYNTAX); return; }
    if (argc < lo || argc > hi) { fail(c, CE_ARG); return; }
    if (body) {
        rotate_body(c, p0, blen);
        if (c->pr->n + 1 > MAX_CODE) { fail(c, CE_STACK); return; }
        /* insert the BLOCK_FN instruction before the body */
        memmove(&c->pr->code[c->pr->n - blen + 1], &c->pr->code[c->pr->n - blen], (size_t)blen * sizeof(ins_t));
        {
            ins_t *i = &c->pr->code[c->pr->n - blen];
            i->op = OP_BLOCK_FN; i->a = (uint8_t)argc; i->x = (uint16_t)ft; i->y = (uint16_t)blen;
        }
        c->pr->n++;
    } else {
        emit(c, OP_FN, (uint8_t)argc, (uint16_t)ft, 0);
    }
}

static void parse_atom(cs_t *c)
{
    int t = peek(c);
    int nests;

    if (c->err) return;
    if (++c->depth > MAX_DEPTH) { fail(c, CE_STACK); return; }
    nests = t == T_LP || t == T_LBRK || t == T_FRAC || t == T_MFRAC || t == T_SQRT || t == T_ROOT || t == T_LOGB || is_fn_tok(t);
    if (nests && ++c->nest > MAX_NEST) { c->nest--; c->depth--; fail(c, CE_STACK); return; }

    if (is_digit_tok(t) || t == T_DOT || t == T_BASEPFX) {
        parse_number_dms(c);
    } else if (t == T_LP) {
        c->p++;
        parse_or(c);
        if (!accept(c, T_RP)) fail(c, CE_SYNTAX);
    } else if (t == T_LBRK) {
        int rows = 1, cols = 1, cur = 1;
        c->p++;
        for (;;) {
            parse_or(c);
            if (c->err) goto out;
            if (accept(c, T_COMMA)) { cur++; continue; }
            if (accept(c, T_SEMI)) {
                if (rows == 1) cols = cur; else if (cur != cols) { fail(c, CE_SYNTAX); goto out; }
                rows++; cur = 1;
                continue;
            }
            break;
        }
        if (!accept(c, T_RBRK)) { fail(c, CE_SYNTAX); goto out; }
        if (rows == 1) cols = cur; else if (cur != cols) { fail(c, CE_SYNTAX); goto out; }
        if (rows > MAT_MAX_R || cols > MAT_MAX_R) { fail(c, CE_DIM); goto out; }
        emit(c, OP_MATLIT, (uint8_t)rows, (uint16_t)cols, 0);
    } else if (t == T_FRAC) {
        c->p++;
        parse_or(c);
        if (!accept(c, T_FRACSEP)) { fail(c, CE_SYNTAX); goto out; }
        parse_or(c);
        if (!accept(c, T_FRACEND)) { fail(c, CE_SYNTAX); goto out; }
        emit(c, T_DIV, 0, 0, 0);
    } else if (t == T_MFRAC) {
        c->p++;
        parse_or(c);
        if (!accept(c, T_MSEP1)) { fail(c, CE_SYNTAX); goto out; }
        parse_or(c);
        if (!accept(c, T_MSEP2)) { fail(c, CE_SYNTAX); goto out; }
        parse_or(c);
        if (!accept(c, T_MEND)) { fail(c, CE_SYNTAX); goto out; }
        emit(c, T_DIV, 0, 0, 0);
        emit(c, T_ADD, 0, 0, 0);
    } else if (t == T_SQRT) {
        c->p++;
        parse_or(c);
        if (!accept(c, T_SQEND)) { fail(c, CE_SYNTAX); goto out; }
        emit(c, OP_FN, 1, T_SQRT, 0);
    } else if (t == T_ROOT) {
        c->p++;
        parse_or(c);
        if (!accept(c, T_ROOTSEP)) { fail(c, CE_SYNTAX); goto out; }
        parse_or(c);
        if (!accept(c, T_ROOTEND)) { fail(c, CE_SYNTAX); goto out; }
        emit(c, OP_FN, 2, T_ROOT, 0);
    } else if (t == T_LOGB) {
        c->p++;
        parse_or(c);
        if (!accept(c, T_COMMA)) { fail(c, CE_SYNTAX); goto out; }
        parse_or(c);
        if (!accept(c, T_RP)) { fail(c, CE_SYNTAX); goto out; }
        emit(c, OP_FN, 2, T_LOG, 0);
    } else if (is_fn_tok(t)) {
        c->p++;
        parse_fn(c, t);
    } else if (t == T_PI || t == T_E || t == T_I || t == T_ANS || t == T_PREANS || t == T_RAN) {
        emit(c, (uint8_t)t, 0, 0, 0);
        c->p++;
    } else if (t == T_CONST || t == T_STATVAR || t == T_VAR || t == T_MAT || t == T_VCT) {
        emit(c, (uint8_t)t, c->t[c->p].a, 0, 0);
        if (t == T_VAR && c->t[c->p].a == VAR_X) c->pr->uses_x = 1;
        c->p++;
    } else {
        fail(c, CE_SYNTAX);
    }
out:
    if (nests) c->nest--;
    c->depth--;
}

static void parse_postfix_ops(cs_t *c)
{
    for (;;) {
        int t = peek(c);
        if (c->err) return;
        switch (t) {
        case T_FACT: case T_SQ: case T_CUBE: case T_RECIP: case T_PERCENT:
        case T_DEGSYM: case T_RADSYM: case T_GRADSYM:
            emit(c, (uint8_t)t, 0, 0, 0); c->p++; break;
        case T_CONV:
            emit(c, T_CONV, c->t[c->p].a, 0, 0); c->p++; break;
        default:
            return;
        }
    }
}

static void parse_unary(cs_t *c);

/* Decimal base-n mode: -2147483648 (the 32-bit minimum) must be enterable although 2147483648 alone is out of
 * range. After a minus sign, the exact digit string 2147483648 is accepted as that one value. */
#if defined(__GNUC__)
__attribute__((noinline))
#endif
static int min32_literal(cs_t *c)
{
    static const uint8_t D[10] = {2, 1, 4, 7, 4, 8, 3, 6, 4, 8};
    int i;
    if (c->p + 10 > c->n) return 0;
    for (i = 0; i < 10; i++) if (c->t[c->p + i].t != T_D0 + D[i]) return 0;
    if (c->p + 10 < c->n && is_digit_tok(c->t[c->p + 10].t)) return 0;
    c->p += 10;
    {
        num_t v;
        num_from_int(&v, -2147483647LL - 1);
        emit_const(c, &v);
    }
    return 1;
}

/* exponent after a text '^': optional sign, then one primary (right associative) */
static void parse_caret_exponent(cs_t *c)
{
    int neg = 0;
    if (++c->depth > MAX_DEPTH) { fail(c, CE_STACK); return; }
    if (accept(c, T_SUB) || accept(c, T_NEG)) neg = 1;
    else accept(c, T_ADD);
    parse_atom(c);
    if (peek(c) == T_FACT) { emit(c, T_FACT, 0, 0, 0); c->p++; }
    if (peek(c) == T_CARET) {
        c->p++;
        parse_caret_exponent(c);
        emit(c, T_POW, 0, 0, 0);
    }
    if (neg) emit(c, OP_UNEG, 0, 0, 0);
    c->depth--;
}

static void parse_power(cs_t *c)
{
    parse_atom(c);
    parse_postfix_ops(c);
    while (!c->err) {
        int t = peek(c);
        if (t == T_CARET) {
            c->p++;
            parse_caret_exponent(c);
        } else if (t == T_POW) {
            c->p++;
            if (++c->nest > MAX_NEST) { c->nest--; fail(c, CE_STACK); return; }
            parse_or(c);
            c->nest--;
            if (!accept(c, T_PEND)) { fail(c, CE_SYNTAX); return; }
        } else {
            break;
        }
        emit(c, T_POW, 0, 0, 0);
        parse_postfix_ops(c);
    }
}

static void parse_unary(cs_t *c)
{
    int t = peek(c);
    if (c->err) return;
    if (t == T_SUB || t == T_NEG) {
        c->p++;
        if (c->base == 10 && min32_literal(c)) return;
        if (++c->depth > MAX_DEPTH) { fail(c, CE_STACK); return; }
        parse_unary(c);
        c->depth--;
        emit(c, OP_UNEG, 0, 0, 0);
    } else if (t == T_ADD) {
        c->p++;
        parse_unary(c);
    } else {
        parse_power(c);
    }
}

static void parse_perm(cs_t *c)
{
    parse_unary(c);
    while (!c->err && (peek(c) == T_NPR || peek(c) == T_NCR)) {
        int t = peek(c);
        c->p++;
        parse_unary(c);
        emit(c, (uint8_t)t, 0, 0, 0);
    }
}

static void parse_implicit(cs_t *c)
{
    parse_perm(c);
    while (!c->err && starts_atom(peek(c))) {
        parse_perm(c);
        emit(c, T_MUL, 0, 0, 0);
    }
}

static void parse_mul(cs_t *c)
{
    parse_implicit(c);
    while (!c->err && (peek(c) == T_MUL || peek(c) == T_DIV)) {
        int t = peek(c);
        c->p++;
        parse_implicit(c);
        emit(c, (uint8_t)t, 0, 0, 0);
    }
}

static void parse_add(cs_t *c)
{
    parse_mul(c);
    while (!c->err && (peek(c) == T_ADD || peek(c) == T_SUB)) {
        int t = peek(c);
        c->p++;
        parse_mul(c);
        emit(c, (uint8_t)t, 0, 0, 0);
    }
}

static void parse_and(cs_t *c)
{
    parse_add(c);
    while (!c->err && peek(c) == T_AND) {
        c->p++;
        parse_add(c);
        emit(c, T_AND, 0, 0, 0);
    }
}

static void parse_or(cs_t *c)
{
    if (++c->depth > MAX_DEPTH) { fail(c, CE_STACK); return; }
    parse_and(c);
    while (!c->err && (peek(c) == T_OR || peek(c) == T_XOR || peek(c) == T_XNOR)) {
        int t = peek(c);
        c->p++;
        parse_and(c);
        emit(c, (uint8_t)t, 0, 0, 0);
    }
    c->depth--;
}

static void parse_stmt(cs_t *c)
{
    parse_or(c);
    if (c->err) return;
    if (accept(c, T_EQ)) {
        parse_or(c);
        emit(c, T_EQ, 0, 0, 0);
        c->pr->has_eq = 1;
    }
    while (!c->err && peek(c) == T_STO) {
        emit(c, T_STO, c->t[c->p].a, 0, 0);
        c->p++;
    }
}

calc_err_t compile_tokens(const tok_t *toks, int ntok, int base, prog_t *p)
{
    cs_t c;
    memset(p, 0, sizeof *p);
    p->base = (uint8_t)base;
    c.t = toks; c.n = ntok; c.p = 0; c.pr = p; c.base = base; c.depth = 0; c.nest = 0; c.err = CE_OK;
    if (ntok == 0) return CE_SYNTAX;
    parse_stmt(&c);                  /* single statement only: no ':' chaining (excluded feature) */
    if (c.err) return c.err;
    if (!c.err && c.p != c.n) c.err = CE_SYNTAX;
    return c.err;
}

calc_err_t compile_text(const char *s, int base, prog_t *p)
{
    tok_t toks[MAX_TOKENS];
    int n = 0;
    calc_err_t e = lex_text(s, base, toks, MAX_TOKENS, &n);
    if (e) { memset(p, 0, sizeof *p); return e; }
    return compile_tokens(toks, n, base, p);
}
