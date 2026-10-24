#include <string.h>
#include "calc/format.h"
#include "values/cplx.h"

#define NDIG 16

static void digits_of(const dec_t *d, uint8_t dg[NDIG], int *exp)
{
    uint64_t m = d->m;
    int i;
    for (i = NDIG - 1; i >= 0; i--) { dg[i] = (uint8_t)(m % 10u); m /= 10u; }
    *exp = d->exp;
}

/* Round dg[0..15] to 'keep' digits (0..16), half up. Returns carry-out (1 if value became 10^k). */
static int round_digits(uint8_t dg[NDIG], int keep)
{
    int i, carry = 0;
    if (keep >= NDIG) return 0;
    if (keep < 0) { memset(dg, 0, NDIG); return 0; }
    carry = dg[keep] >= 5;
    for (i = keep; i < NDIG; i++) dg[i] = 0;
    for (i = keep - 1; i >= 0 && carry; i--) {
        if (dg[i] == 9) dg[i] = 0; else { dg[i]++; carry = 0; }
    }
    if (carry) {
        /* all digits were 9: becomes 1000... */
        memset(dg, 0, NDIG);
        dg[0] = 1;
        return 1;
    }
    return 0;
}

dec_status dec_round_sig(dec_t *r, const dec_t *x, int sig)
{
    uint8_t dg[NDIG];
    int e, i, carry;
    uint64_t m = 0;
    dec_t t;
    if (dec_is_zero(x) || sig >= NDIG) { *r = *x; return DEC_OK; }
    if (sig < 1) sig = 1;
    digits_of(x, dg, &e);
    carry = round_digits(dg, sig);
    if (carry) e++;
    if (e > DEC_EXP_MAX) { dec_zero(r); return DEC_OVERFLOW; }
    for (i = 0; i < NDIG; i++) m = m * 10u + dg[i];
    t.m = m; t.exp = (int16_t)e; t.neg = x->neg;
    *r = t;
    return DEC_OK;
}

/* Build digit text for a number in plain notation. dg has 'keep' significant digits. */
static void plain_text(const uint8_t *dg, int keep, int e, int strip, char *out, size_t cap)
{
    char tmp[64];
    int n = 0, i;
    int last = keep;
    if (strip) while (last > 1 && dg[last - 1] == 0) last--;
    if (e >= 0) {
        for (i = 0; i <= e; i++) tmp[n++] = (char)('0' + (i < keep ? dg[i] : 0));
        if (last > e + 1) {
            tmp[n++] = '.';
            for (i = e + 1; i < last; i++) tmp[n++] = (char)('0' + dg[i]);
        }
    } else {
        tmp[n++] = '0'; tmp[n++] = '.';
        for (i = 0; i < -e - 1; i++) tmp[n++] = '0';
        for (i = 0; i < last; i++) tmp[n++] = (char)('0' + dg[i]);
    }
    if ((size_t)n >= cap) n = (int)cap - 1;
    memcpy(out, tmp, (size_t)n);
    out[n] = 0;
}

static void mant_text(const uint8_t *dg, int keep, int strip, char *out, size_t cap)
{
    /* d.ddd */
    char tmp[32];
    int n = 0, i, last = keep;
    if (strip) while (last > 1 && dg[last - 1] == 0) last--;
    tmp[n++] = (char)('0' + dg[0]);
    if (last > 1) {
        tmp[n++] = '.';
        for (i = 1; i < last; i++) tmp[n++] = (char)('0' + dg[i]);
    }
    if ((size_t)n >= cap) n = (int)cap - 1;
    memcpy(out, tmp, (size_t)n);
    out[n] = 0;
}

static void int_to_str(int v, char *out)
{
    char t[12];
    int n = 0, neg = v < 0, i;
    unsigned u = (unsigned)(neg ? -v : v);
    if (u == 0) t[n++] = '0';
    while (u) { t[n++] = (char)('0' + u % 10u); u /= 10u; }
    i = 0;
    if (neg) out[i++] = '-';
    while (n) out[i++] = t[--n];
    out[i] = 0;
}

static void fmt_dec_part(const ctx_t *c, const dec_t *x, fmt_part_t *p)
{
    uint8_t dg[NDIG];
    int e, keep, carry, sci = 0, eng = 0, strip = 1;

    memset(p, 0, sizeof *p);
    if (dec_is_zero(x)) {
        if (c->fmt == FMT_FIX && c->fmt_digits > 0) {
            int i, n = 0;
            p->num[n++] = '0'; p->num[n++] = '.';
            for (i = 0; i < c->fmt_digits; i++) p->num[n++] = '0';
            p->num[n] = 0;
        } else if (c->fmt == FMT_SCI || c->fmt == FMT_ENG) {
            int i, n = 0, nd = c->fmt == FMT_SCI ? (c->fmt_digits ? c->fmt_digits : 9) : 0;
            p->num[n++] = '0';
            if (nd) { p->num[n++] = '.'; for (i = 0; i < nd; i++) p->num[n++] = '0'; }
            p->num[n] = 0;
            int_to_str(0, p->exp);
        } else {
            p->num[0] = '0'; p->num[1] = 0;
        }
        return;
    }
    p->neg = (char)x->neg;
    digits_of(x, dg, &e);

    switch (c->fmt) {
    case FMT_SCI: {
        int nd = c->fmt_digits ? c->fmt_digits + 1 : 10;
        carry = round_digits(dg, nd);
        if (carry) e++;
        mant_text(dg, nd, 0, p->num, sizeof p->num);
        int_to_str(e, p->exp);
        return;
    }
    case FMT_ENG: {
        int m3 = ((e % 3) + 3) % 3, ee, kd;
        carry = round_digits(dg, c->fmt_digits ? c->fmt_digits + 1 : 10);
        if (carry) e++;
        m3 = ((e % 3) + 3) % 3;
        ee = e - m3;
        kd = c->fmt_digits ? c->fmt_digits + 1 : 10;
        /* mantissa has m3+1 integer digits */
        {
            uint8_t sh[NDIG];
            memcpy(sh, dg, NDIG);
            plain_text(sh, kd, m3, c->fmt_digits == 0, p->num, sizeof p->num);
        }
        int_to_str(ee, p->exp);
        return;
    }
    case FMT_FIX: {
        keep = e + 1 + c->fmt_digits;
        if (e >= 10) sci = 1;
        else {
            if (keep < 0) {
                /* rounds to zero */
                int i, n = 0;
                p->neg = 0;
                p->num[n++] = '0';
                if (c->fmt_digits) { p->num[n++] = '.'; for (i = 0; i < c->fmt_digits; i++) p->num[n++] = '0'; }
                p->num[n] = 0;
                return;
            }
            if (keep > NDIG) keep = NDIG;
            carry = round_digits(dg, keep);
            if (carry) { e++; keep++; if (keep > NDIG) keep = NDIG; }
            if (keep == 0) {
                /* rounded to 0 or to 10^-fmt_digits */
                int i, n = 0;
                if (!carry) {
                    p->neg = 0;
                    p->num[n++] = '0';
                    if (c->fmt_digits) { p->num[n++] = '.'; for (i = 0; i < c->fmt_digits; i++) p->num[n++] = '0'; }
                    p->num[n] = 0;
                    return;
                }
            }
            if (e >= 10) sci = 1;
            else {
                /* render with exactly fmt_digits decimals */
                char tmp[48];
                int n = 0, i, intd = e >= 0 ? e + 1 : 1;
                if (e >= 0) for (i = 0; i <= e; i++) tmp[n++] = (char)('0' + dg[i]);
                else tmp[n++] = '0';
                if (c->fmt_digits) {
                    tmp[n++] = '.';
                    for (i = 0; i < c->fmt_digits; i++) {
                        int idx = (e >= 0) ? e + 1 + i : i + e + 1;
                        /* for e<0 the first fractional digit corresponds to dg[0] at position -e-1 */
                        int d = 0;
                        if (e >= 0) d = idx < NDIG ? dg[idx] : 0;
                        else { int k = i + e + 1; d = (k >= 0 && k < NDIG) ? dg[k] : 0; }
                        tmp[n++] = (char)('0' + d);
                    }
                }
                (void)intd;
                tmp[n] = 0;
                strcpy(p->num, tmp);
                return;
            }
        }
        break;
    }
    default:
        break;
    }

    if (c->fmt == FMT_FIX && sci) {
        keep = 10; strip = 1;
        carry = round_digits(dg, keep);
        if (carry) e++;
        mant_text(dg, keep, strip, p->num, sizeof p->num);
        int_to_str(e, p->exp);
        return;
    }

    /* Norm 1 / Norm 2 */
    keep = 10;
    carry = round_digits(dg, keep);
    if (carry) e++;
    if (c->fmt == FMT_NORM2) eng = (e < -9 || e > 9);
    else eng = (e < -2 || e > 9);
    if (eng) {
        mant_text(dg, keep, 1, p->num, sizeof p->num);
        int_to_str(e, p->exp);
    } else {
        plain_text(dg, keep, e, 1, p->num, sizeof p->num);
    }
    (void)sci;
}

static int ndigits_str(const char *s)
{
    int n = 0;
    for (; *s; s++) if (*s >= '0' && *s <= '9') n++;
    return n;
}

static void fmt_real_inner(const ctx_t *c, const num_t *n, int exact, fmt_res_t *out)
{
    memset(out, 0, sizeof *out);
    if (exact && n->ex.kind != EX_NONE && ex_valid(&n->ex)) {
        ex_text_t t;
        ex_format(&n->ex, 0, &t);
        if (n->ex.kind == EX_QS && n->ex.b == 0 && n->ex.d == 1) {
            /* integer: decimal path is exact enough up to 10 digits */
        } else {
            int nd = ndigits_str(t.num), dd = ndigits_str(t.den);
            int rat = (n->ex.kind == EX_QS && n->ex.b == 0);
            if (rat) {
                /* fraction */
                int improper = c->frac_improper;
                int64_t a = n->ex.a < 0 ? -n->ex.a : n->ex.a, d = n->ex.d;
                int64_t whole = a / d, rem = a % d;
                if (!improper && whole > 0) {
                    char w[24], r[24];
                    int k = 0; int64_t tw = whole, tr = rem;
                    char tmp[24]; int nn = 0;
                    (void)k;
                    if (tw == 0) tmp[nn++] = '0';
                    while (tw) { tmp[nn++] = (char)('0' + tw % 10); tw /= 10; }
                    { int i; for (i = 0; i < nn; i++) w[i] = tmp[nn - 1 - i]; w[nn] = 0; }
                    nn = 0;
                    while (tr) { tmp[nn++] = (char)('0' + tr % 10); tr /= 10; }
                    { int i; for (i = 0; i < nn; i++) r[i] = tmp[nn - 1 - i]; r[nn] = 0; }
                    if (ndigits_str(w) + ndigits_str(r) + dd + 2 <= 10) {
                        out->kind = FR_MIXED;
                        out->p.neg = t.neg;
                        strcpy(out->p.whole, w);
                        strcpy(out->p.num, r);
                        strcpy(out->p.den, t.den);
                        return;
                    }
                } else if (nd + dd + 1 <= 10) {
                    out->kind = FR_FRAC;
                    out->p.neg = t.neg;
                    strcpy(out->p.num, t.num);
                    strcpy(out->p.den, t.den);
                    return;
                }
            } else if (nd <= 10 && dd <= 10) {
                out->kind = FR_EXACT;
                out->p.neg = t.neg;
                strcpy(out->p.num, t.num);
                strcpy(out->p.den, t.den);
                return;
            }
        }
    }
    out->kind = FR_DEC;
    fmt_dec_part(c, &n->v, &out->p);
}

void fmt_real(const ctx_t *c, const num_t *n, int exact, fmt_res_t *out)
{
    fmt_real_inner(c, n, exact, out);
    out->p.kind = out->kind;
}

void fmt_value(const ctx_t *c, const val_t *v, int exact, fmt_res_t *out)
{
    if (v->k == V_CPLX) {
        fmt_res_t a, b;
        num_t t;
        if (c->polar) {
            cx_t z;
            num_t mag, ang;
            z.re = v->re; z.im = v->im;
            if (cx_abs(&mag, &z) == DEC_OK && cx_arg(&ang, &z, c->angle) == DEC_OK) {
                fmt_real(c, &mag, exact, &a);
                fmt_real(c, &ang, exact, &b);
                memset(out, 0, sizeof *out);
                out->kind = FR_CPLX;
                out->polar = 1;
                out->p = a.p;
                out->q = b.p;
                return;
            }
        }
        fmt_real(c, &v->re, exact, &a);
        t = v->im;
        fmt_real(c, &t, exact, &b);
        if (exact && !num_is_zero(&v->re) && (a.kind == FR_DEC) != (b.kind == FR_DEC)) {
            /* one part is exact and the other is not: show both as decimals */
            fmt_real(c, &v->re, 0, &a);
            fmt_real(c, &t, 0, &b);
        }
        memset(out, 0, sizeof *out);
        out->kind = FR_CPLX;
        out->p = a.p;
        out->q = b.p;
        out->re_zero = (uint8_t)num_is_zero(&v->re);
        {
            num_t one, ab;
            num_from_int(&one, 1);
            num_abs(&ab, &v->im);
            out->im_one = (uint8_t)(num_cmp(&ab, &one) == 0);
        }
        return;
    }
    fmt_real(c, &v->re, exact, out);
}

static size_t put(char *buf, size_t cap, size_t pos, const char *s)
{
    size_t n = strlen(s);
    if (pos + n >= cap) n = cap > pos + 1 ? cap - pos - 1 : 0;
    memcpy(buf + pos, s, n);
    buf[pos + n] = 0;
    return pos + n;
}

static const char *ascii_fix(const char *s, char *tmp, size_t cap)
{
    /* replace UTF-8 sqrt / pi by ascii */
    size_t i = 0, o = 0;
    while (s[i] && o + 8 < cap) {
        if ((unsigned char)s[i] == 0xe2 && (unsigned char)s[i + 1] == 0x88 && (unsigned char)s[i + 2] == 0x9a) {
            memcpy(tmp + o, "sqrt", 4); o += 4; i += 3;
            /* wrap following run of digits in parentheses */
            if (s[i] >= '0' && s[i] <= '9') {
                tmp[o++] = '(';
                while (s[i] >= '0' && s[i] <= '9') tmp[o++] = s[i++];
                tmp[o++] = ')';
            }
        } else if ((unsigned char)s[i] == 0xcf && (unsigned char)s[i + 1] == 0x80) {
            memcpy(tmp + o, "pi", 2); o += 2; i += 2;
        } else {
            tmp[o++] = s[i++];
        }
    }
    tmp[o] = 0;
    return tmp;
}

static size_t part_text(const fmt_part_t *p, int skip_sign, int ascii, char *buf, size_t cap)
{
    size_t pos = 0;
    char t1[96], t2[48];
    if (cap == 0) return 0;
    buf[0] = 0;
    if (p->neg && !skip_sign) pos = put(buf, cap, pos, "-");
    switch (p->kind) {
    case FR_MIXED:
        pos = put(buf, cap, pos, p->whole);
        pos = put(buf, cap, pos, " ");
        pos = put(buf, cap, pos, p->num);
        pos = put(buf, cap, pos, "/");
        pos = put(buf, cap, pos, p->den);
        break;
    case FR_FRAC:
        pos = put(buf, cap, pos, p->num);
        pos = put(buf, cap, pos, "/");
        pos = put(buf, cap, pos, p->den);
        break;
    case FR_EXACT: {
        const char *nm = ascii ? ascii_fix(p->num, t1, sizeof t1) : p->num;
        int multi = p->den[0] && (strchr(p->num, '+') || strchr(p->num, '-'));
        if (multi) pos = put(buf, cap, pos, "(");
        pos = put(buf, cap, pos, nm);
        if (multi) pos = put(buf, cap, pos, ")");
        if (p->den[0]) {
            pos = put(buf, cap, pos, "/");
            pos = put(buf, cap, pos, ascii ? ascii_fix(p->den, t2, sizeof t2) : p->den);
        }
        break;
    }
    default:
        pos = put(buf, cap, pos, p->num);
        if (p->exp[0]) {
            pos = put(buf, cap, pos, "E");
            pos = put(buf, cap, pos, p->exp);
        }
        break;
    }
    return pos;
}

size_t fmt_text(const fmt_res_t *r, int ascii, char *buf, size_t cap)
{
    size_t pos;
    char tmp[128];
    if (r->kind != FR_CPLX) return part_text(&r->p, 0, ascii, buf, cap);
    buf[0] = 0;
    if (r->polar) {
        pos = part_text(&r->p, 0, ascii, buf, cap);
        pos = put(buf, cap, pos, ascii ? "<" : "\xe2\x88\xa0");
        part_text(&r->q, 0, ascii, tmp, sizeof tmp);
        return put(buf, cap, pos, tmp);
    }
    pos = 0;
    if (!r->re_zero) pos = put(buf, cap, pos, "") , pos = part_text(&r->p, 0, ascii, buf, cap);
    if (r->q.neg) pos = put(buf, cap, pos, "-");
    else if (!r->re_zero) pos = put(buf, cap, pos, "+");
    if (!r->im_one) {
        part_text(&r->q, 1, ascii, tmp, sizeof tmp);
        if (r->q.kind == FR_EXACT && strchr(tmp, '+')) { pos = put(buf, cap, pos, "("); pos = put(buf, cap, pos, tmp); pos = put(buf, cap, pos, ")"); }
        else pos = put(buf, cap, pos, tmp);
    }
    {
        size_t l = strlen(buf);
        if (ascii && l >= 2 && buf[l - 2] == 'p' && buf[l - 1] == 'i') pos = put(buf, cap, pos, " ");
    }
    return put(buf, cap, pos, "i");
}

size_t fmt_value_text(const ctx_t *c, const val_t *v, int exact, int ascii, char *buf, size_t cap)
{
    fmt_res_t r;
    if (v->k == V_MAT || v->k == V_VCT) {
        const num_t *e;
        int rows, cols, i, j;
        size_t pos = 0;
        char cell[64];
        ctx_t cc = *c;
        cc.frac_improper = 1;   /* matrix cells use improper fractions */
        if (v->k == V_MAT) { rows = c->mats[v->mat].r; cols = c->mats[v->mat].c; e = c->mats[v->mat].e; }
        else { rows = c->vcts[v->mat].r; cols = 1; e = c->vcts[v->mat].e; }
        if (cap == 0) return 0;
        buf[0] = 0;
        pos = put(buf, cap, pos, v->k == V_VCT ? "Vct[" : "[");
        for (i = 0; i < rows; i++) {
            if (v->k == V_MAT) pos = put(buf, cap, pos, i ? ", [" : "[");
            for (j = 0; j < cols; j++) {
                fmt_real(&cc, &e[i * cols + j], exact, &r);
                fmt_text(&r, ascii, cell, sizeof cell);
                if (j) pos = put(buf, cap, pos, ", ");
                else if (v->k == V_VCT && i) pos = put(buf, cap, pos, ", ");
                pos = put(buf, cap, pos, cell);
            }
            if (v->k == V_MAT) pos = put(buf, cap, pos, "]");
        }
        return put(buf, cap, pos, "]");
    }
    fmt_value(c, v, exact, &r);
    return fmt_text(&r, ascii, buf, cap);
}

dec_status dec_round_display(const ctx_t *c, dec_t *r, const dec_t *x)
{
    uint8_t dg[NDIG];
    int e, keep;
    if (dec_is_zero(x)) { *r = *x; return DEC_OK; }
    digits_of(x, dg, &e);
    switch (c->fmt) {
    case FMT_FIX:
        keep = e + 1 + c->fmt_digits;
        if (keep < 0) { dec_zero(r); return DEC_OK; }
        if (keep == 0) {
            if (dg[0] < 5) { dec_zero(r); return DEC_OK; }
            /* rounds up to 10^-fmt_digits */
            { dec_t one, p; dec_from_i64(&one, 1); dec_zero(&p);
              /* 10^-n via parse */
              { char s[16]; int i, k = 0; s[k++]='1'; s[k++]='E'; s[k++]='-'; s[k++]=(char)('0'+c->fmt_digits); s[k]=0; (void)i; dec_from_str(&p, s); }
              *r = p; r->neg = x->neg; return DEC_OK; }
        }
        return dec_round_sig(r, x, keep);
    case FMT_SCI: case FMT_ENG:
        return dec_round_sig(r, x, c->fmt_digits ? c->fmt_digits + 1 : 10);
    default:
        return dec_round_sig(r, x, 10);
    }
}
