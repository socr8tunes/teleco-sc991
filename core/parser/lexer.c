#include <string.h>
#include "parser/lexer.h"

typedef struct word {
    const char *name;
    uint8_t tok;
    uint8_t kind;   /* 0 function group "name(", 1 atom, 2 infix word, 3 postfix word */
} word_t;

enum { W_FN = 0, W_ATOM = 1, W_INFIX = 2, W_POST = 3 };

static const word_t WORDS[] = {
    {"sin", T_SIN, W_FN}, {"cos", T_COS, W_FN}, {"tan", T_TAN, W_FN},
    {"asin", T_ASIN, W_FN}, {"acos", T_ACOS, W_FN}, {"atan", T_ATAN, W_FN},
    {"sinh", T_SINH, W_FN}, {"cosh", T_COSH, W_FN}, {"tanh", T_TANH, W_FN},
    {"asinh", T_ASINH, W_FN}, {"acosh", T_ACOSH, W_FN}, {"atanh", T_ATANH, W_FN},
    {"log", T_LOG, W_FN}, {"ln", T_LN, W_FN}, {"exp", T_EXP, W_FN},
    {"abs", T_ABS, W_FN}, {"cbrt", T_CBRT, W_FN},
    {"int", T_INT, W_FN}, {"intg", T_INTG, W_FN}, {"rnd", T_RND, W_FN},
    {"ranint", T_RANINT, W_FN}, {"pol", T_POL, W_FN}, {"rec", T_REC, W_FN},
    {"arg", T_ARG, W_FN}, {"conj", T_CONJ, W_FN}, {"re", T_RE, W_FN}, {"im", T_IM, W_FN},
    {"sum", T_SIGMA, W_FN}, {"ddx", T_DDX, W_FN}, {"integral", T_INTEGRAL, W_FN},
    {"det", T_DET, W_FN}, {"trn", T_TRN, W_FN}, {"iden", T_IDEN, W_FN},
    {"dot", T_DOTP, W_FN}, {"cross", T_CROSSP, W_FN},
    {"anglev", T_ANGLEV, W_FN}, {"unitv", T_UNITV, W_FN},
    {"solve", T_SOLVE, W_FN}, {"not", T_NOT, W_FN}, {"neg", T_NEGB, W_FN},
    {"sqrt", T_SQRT, W_FN}, {"root", T_ROOT, W_FN},
    {"frac", T_FRAC, W_FN}, {"mfrac", T_MFRAC, W_FN},
    {"pi", T_PI, W_ATOM}, {"ans", T_ANS, W_ATOM}, {"preans", T_PREANS, W_ATOM},
    {"ran#", T_RAN, W_ATOM},
    {"npr", T_NPR, W_INFIX}, {"ncr", T_NCR, W_INFIX},
    {"and", T_AND, W_INFIX}, {"or", T_OR, W_INFIX},
    {"xor", T_XOR, W_INFIX}, {"xnor", T_XNOR, W_INFIX},
    {"deg", T_DEGSYM, W_POST}, {"rad", T_RADSYM, W_POST}, {"grad", T_GRADSYM, W_POST},
};
#define NWORDS (int)(sizeof WORDS / sizeof WORDS[0])

typedef struct grp {
    uint8_t close;   /* token emitted at ')' */
    uint8_t comma;   /* token emitted at ',' */
} grp_t;

typedef struct lx {
    const char *s;
    int p;
    tok_t *out;
    int cap, n;
    grp_t g[MAX_DEPTH];
    int ng;
    calc_err_t err;
} lx_t;

static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
static int is_digit(int c) { return c >= '0' && c <= '9'; }
static int is_alpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

static void emit(lx_t *l, uint8_t t, uint8_t a)
{
    if (l->err) return;
    if (l->n >= l->cap) { l->err = CE_STACK; return; }
    l->out[l->n++] = mk_tok(t, a);
}

static void push_group(lx_t *l, uint8_t close, uint8_t comma)
{
    if (l->err) return;
    if (l->ng >= MAX_DEPTH) { l->err = CE_STACK; return; }
    l->g[l->ng].close = close;
    l->g[l->ng].comma = comma;
    l->ng++;
}

static int match_word(const char *s, const char *name)
{
    int i = 0;
    while (name[i]) {
        if (lower((unsigned char)s[i]) != name[i]) return 0;
        i++;
    }
    return i;
}

/* UTF-8 symbol test: returns byte length if s starts with the sequence. */
static int sym(const char *s, const char *seq)
{
    size_t n = strlen(seq);
    return strncmp(s, seq, n) == 0 ? (int)n : 0;
}

static int var_index(int c)
{
    switch (lower(c)) {
    case 'a': return VAR_A; case 'b': return VAR_B; case 'c': return VAR_C;
    case 'd': return VAR_D; case 'f': return VAR_F;
    case 'x': return VAR_X; case 'y': return VAR_Y; case 'm': return VAR_M;
    default: return -1;
    }
}

static void lex_number(lx_t *l)
{
    const char *s = l->s;
    while (is_digit(s[l->p]) || s[l->p] == '.') {
        emit(l, s[l->p] == '.' ? T_DOT : (uint8_t)(T_D0 + (s[l->p] - '0')), 0);
        l->p++;
    }
    /* exponent: E or e followed by [sign] digit */
    if (lower((unsigned char)s[l->p]) == 'e') {
        int q = l->p + 1;
        if (s[q] == '-' || s[q] == '+' || sym(s + q, "\xe2\x88\x92")) {
            q += (s[q] == '-' || s[q] == '+') ? 1 : 3;
        }
        if (is_digit(s[q])) {
            int neg = 0, k;
            emit(l, T_EE, 0);
            for (k = l->p + 1; k < q; ) {
                if (s[k] == '-' || s[k] == (char)0xe2) neg = 1;
                k += (s[k] == (char)0xe2) ? 3 : 1;
            }
            if (neg) emit(l, T_NEG, 0);
            l->p = q;
            while (is_digit(s[l->p])) { emit(l, (uint8_t)(T_D0 + (s[l->p] - '0')), 0); l->p++; }
        }
    }
}

/* Lex the single operand following a root sign without parentheses. */
static void lex_root_operand(lx_t *l)
{
    const char *s = l->s;
    int c = (unsigned char)s[l->p];
    if (is_digit(c) || c == '.') { lex_number(l); return; }
    if (sym(s + l->p, "\xcf\x80")) { emit(l, T_PI, 0); l->p += 2; return; }
    if (is_alpha(c)) {
        int v = var_index(c);
        if (c == 'e') emit(l, T_E, 0);
        else if (c == 'E') emit(l, T_VAR, VAR_E);
        else if (lower(c) == 'i') emit(l, T_I, 0);
        else if (v >= 0) emit(l, T_VAR, (uint8_t)v);
        else { l->err = CE_SYNTAX; return; }
        l->p++;
        return;
    }
    l->err = CE_SYNTAX;
}

calc_err_t lex_text(const char *str, tok_t *out, int cap, int *n)
{
    lx_t L, *l = &L;
    memset(l, 0, sizeof *l);
    l->s = str; l->out = out; l->cap = cap;

    while (!l->err && str[l->p]) {
        const char *s = str + l->p;
        int c = (unsigned char)*s, k;

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { l->p++; continue; }
        if (is_digit(c) || c == '.') {
            /* "2 3" must not read as 23 */
            if (l->n > 0 && l->p > 0 && l->s[l->p - 1] == ' ') {
                int lt = l->out[l->n - 1].t;
                if ((lt >= T_D0 && lt <= T_D9) || lt == T_DOT) { l->err = CE_SYNTAX; break; }
            }
            lex_number(l);
            continue;
        }

        if (c == '+') { emit(l, T_ADD, 0); l->p++; continue; }
        if (c == '-' && s[1] != '>') { emit(l, T_SUB, 0); l->p++; continue; }
        if ((k = sym(s, "\xe2\x88\x92"))) { emit(l, T_SUB, 0); l->p += k; continue; }
        if (c == '*') { emit(l, T_MUL, 0); l->p++; continue; }
        if ((k = sym(s, "\xc3\x97"))) { emit(l, T_MUL, 0); l->p += k; continue; }
        if (c == '/') { emit(l, T_DIV, 0); l->p++; continue; }
        if ((k = sym(s, "\xc3\xb7"))) { emit(l, T_DIV, 0); l->p += k; continue; }
        if (c == '^') { emit(l, T_CARET, 0); l->p++; continue; }
        if (c == '!') { emit(l, T_FACT, 0); l->p++; continue; }
        if (c == '%') { emit(l, T_PERCENT, 0); l->p++; continue; }
        if ((k = sym(s, "\xc2\xb2"))) { emit(l, T_SQ, 0); l->p += k; continue; }
        if ((k = sym(s, "\xc2\xb3"))) { emit(l, T_CUBE, 0); l->p += k; continue; }
        if ((k = sym(s, "\xc2\xb0"))) { emit(l, T_DMS_D, 0); l->p += k; continue; }
        if (c == '\'') { emit(l, T_DMS_M, 0); l->p++; continue; }
        if (c == '"') { emit(l, T_DMS_S, 0); l->p++; continue; }
        if (c == '=') { emit(l, T_EQ, 0); l->p++; continue; }
        if (c == ':') { emit(l, T_COLON, 0); l->p++; continue; }
        if ((k = sym(s, "\xcf\x80"))) { emit(l, T_PI, 0); l->p += k; continue; }
        if ((k = sym(s, "\xe2\x88\x9a"))) {
            l->p += k;
            emit(l, T_SQRT, 0);
            if (str[l->p] == '(') { l->p++; push_group(l, T_SQEND, T_COMMA); }
            else { lex_root_operand(l); emit(l, T_SQEND, 0); }
            continue;
        }
        if (c == '(') { emit(l, T_LP, 0); push_group(l, T_RP, T_COMMA); l->p++; continue; }
        if (c == ')') {
            if (l->ng == 0) { l->err = CE_SYNTAX; break; }
            l->ng--;
            emit(l, l->g[l->ng].close, 0);
            l->p++;
            continue;
        }
        if (c == ',') {
            if (l->ng == 0) { l->err = CE_SYNTAX; break; }
            emit(l, l->g[l->ng - 1].comma, 0);
            if (l->g[l->ng - 1].comma == T_MSEP1) l->g[l->ng - 1].comma = T_MSEP2;
            l->p++;
            continue;
        }
        if ((k = sym(s, "\xe2\x86\x92")) || (k = sym(s, "->"))) {
            int v;
            l->p += k;
            while (str[l->p] == ' ') l->p++;
            v = var_index((unsigned char)str[l->p]);
            if (v < 0) { l->err = CE_SYNTAX; break; }
            emit(l, T_STO, (uint8_t)v);
            l->p++;
            continue;
        }
        if (is_alpha(c)) {
            int best = -1, bl = 0, i;
            for (i = 0; i < NWORDS; i++) {
                int m = match_word(s, WORDS[i].name);
                if (m > bl) { bl = m; best = i; }
            }
            /* 'ran#' ends in '#': only matched via the table */
            if (best >= 0) {
                const word_t *w = &WORDS[best];
                int q = l->p + bl;
                if (w->kind == W_FN) {
                    while (str[q] == ' ') q++;
                    if (str[q] != '(') { l->err = CE_SYNTAX; break; }
                    l->p = q + 1;
                    switch (w->tok) {
                    case T_SQRT: emit(l, T_SQRT, 0); push_group(l, T_SQEND, T_COMMA); break;
                    case T_ROOT: emit(l, T_ROOT, 0); push_group(l, T_ROOTEND, T_ROOTSEP); break;
                    case T_FRAC: emit(l, T_FRAC, 0); push_group(l, T_FRACEND, T_FRACSEP); break;
                    case T_MFRAC: emit(l, T_MFRAC, 0); push_group(l, T_MEND, T_MSEP1); break;
                    default: emit(l, w->tok, 0); push_group(l, T_RP, T_COMMA); break;
                    }
                    continue;
                }
                l->p = q;
                emit(l, w->tok, 0);
                continue;
            }
            if (c == 'e') { emit(l, T_E, 0); l->p++; continue; }
            if (c == 'E') { emit(l, T_VAR, VAR_E); l->p++; continue; }
            if (lower(c) == 'i') { emit(l, T_I, 0); l->p++; continue; }
            k = var_index(c);
            if (k >= 0) { emit(l, T_VAR, (uint8_t)k); l->p++; continue; }
            l->err = CE_SYNTAX;
            break;
        }
        l->err = CE_SYNTAX;
    }
    if (!l->err) {
        /* auto-close; mfrac second comma is rewritten below */
        while (l->ng > 0) { l->ng--; emit(l, l->g[l->ng].close, 0); }
    }
    *n = l->n;
    return l->err;
}
