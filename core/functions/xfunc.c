#include "functions/xfunc.h"
#include "numeric/xconst.h"

/* ------------------------------------------------------------------------ */
/* small helpers                                                            */
/* ------------------------------------------------------------------------ */

static void xn_one(xnum_t *r)
{
    xn_from_u64(r, 1);
}

/* |term| <= 10^-45 * |sum| */
static int negligible(const xnum_t *term, const xnum_t *sum)
{
    return term->l[0] == 0 || sum->l[0] == 0 ? term->l[0] == 0 : term->e <= sum->e - (XN_LIMBS + 1);
}

static void xn_set_huge(xnum_t *r, int tiny)
{
    xn_zero(r);
    r->l[0] = 1;
    r->e = tiny ? -XN_EMAX : XN_EMAX;
}

/* 2 * atanh(z) = ln((1+z)/(1-z)), series valid for |z| <= ~0.3 */
static void two_atanh_series(xnum_t *r, const xnum_t *z)
{
    xnum_t z2, term, sum, t;
    uint32_t n;

    if (z->l[0] == 0) {
        xn_zero(r);
        return;
    }
    xn_mul(&z2, z, z);
    term = *z;
    sum = *z;
    for (n = 1; n < 80; n++) {
        xn_mul(&term, &term, &z2);
        xn_div_u32(&t, &term, 2u * n + 1u);
        xn_add(&sum, &sum, &t);
        if (negligible(&t, &sum)) {
            break;
        }
    }
    xn_mul_u32(r, &sum, 2);
}

/* ------------------------------------------------------------------------ */
/* exp, ln                                                                  */
/* ------------------------------------------------------------------------ */

dec_status xf_exp(xnum_t *r, const xnum_t *x)
{
    xnum_t lim, kk, t, y, term, sum, p10;
    int64_t k;
    uint32_t n;
    int i;

    if (x->l[0] == 0) {
        xn_one(r);
        return DEC_OK;
    }
    xn_from_u64(&lim, 1000);
    if (xn_cmp_abs(x, &lim) > 0) {
        xn_set_huge(r, x->neg);
        return DEC_OK;
    }

    xn_mul(&t, x, &XC_INV_LN10);
    (void)xn_to_i64(&t, XN_ROUND, &k);
    xn_from_i64(&kk, k);
    xn_mul(&t, &kk, &XC_LN10);
    xn_sub(&y, x, &t);          /* |y| <= ln(10)/2 */
    xn_div_u32(&y, &y, 64);

    xn_one(&sum);
    term = sum;
    for (n = 1; n < 60; n++) {
        xn_mul(&term, &term, &y);
        xn_div_u32(&term, &term, n);
        xn_add(&sum, &sum, &term);
        if (negligible(&term, &sum)) {
            break;
        }
    }
    for (i = 0; i < 6; i++) {
        xn_mul(&sum, &sum, &sum);
    }
    xn_pow10(&p10, (int32_t)k);
    xn_mul(r, &sum, &p10);
    return DEC_OK;
}

dec_status xf_ln(xnum_t *r, const xnum_t *x)
{
    xnum_t lo, hi, t, z, num, den, m, p10, res, tmp;
    int32_t dexp;
    int j;

    if (x->l[0] == 0 || x->neg) {
        return DEC_DOMAIN;
    }
    /* direct series for x in [0.75, 1.5): full relative accuracy near 1 */
    xn_from_u64(&hi, 3);
    xn_div_u32(&hi, &hi, 2);       /* 1.5 */
    xn_from_u64(&lo, 3);
    xn_div_u32(&lo, &lo, 4);       /* 0.75 */
    if (xn_cmp(x, &lo) >= 0 && xn_cmp(x, &hi) < 0) {
        xn_one(&t);
        xn_sub(&num, x, &t);
        xn_add(&den, x, &t);
        xn_div(&z, &num, &den);
        two_atanh_series(r, &z);
        return DEC_OK;
    }

    dexp = xn_dec_exponent(x);
    xn_pow10(&p10, -dexp);
    xn_mul(&m, x, &p10);           /* m in [1, 10) (up to rounding) */
    xn_from_u64(&t, 6);
    if (xn_cmp(&m, &t) >= 0) {
        j = 3;
    } else {
        xn_from_u64(&t, 3);
        if (xn_cmp(&m, &t) >= 0) {
            j = 2;
        } else {
            xn_from_u64(&t, 3);
            xn_div_u32(&t, &t, 2);
            j = (xn_cmp(&m, &t) >= 0) ? 1 : 0;
        }
    }
    if (j > 0) {
        xn_div_u32(&m, &m, 1u << j);
    }
    xn_one(&t);
    xn_sub(&num, &m, &t);
    xn_add(&den, &m, &t);
    xn_div(&z, &num, &den);
    two_atanh_series(&res, &z);

    xn_from_i64(&t, dexp);
    xn_mul(&tmp, &t, &XC_LN10);
    xn_add(&res, &res, &tmp);
    if (j > 0) {
        xn_mul_u32(&tmp, &XC_LN2, (uint32_t)j);
        xn_add(&res, &res, &tmp);
    }
    *r = res;
    return DEC_OK;
}

dec_status xf_log1p(xnum_t *r, const xnum_t *x)
{
    xnum_t one, lim, t, num, den, z;

    xn_one(&one);
    xn_add(&t, x, &one);
    if (t.l[0] == 0 || t.neg) {
        return DEC_DOMAIN;
    }
    xn_from_u64(&lim, 1);
    xn_div_u32(&lim, &lim, 4);     /* 0.25 */
    if (xn_cmp_abs(x, &lim) < 0) {
        xn_from_u64(&num, 2);
        xn_add(&den, x, &num);     /* 2 + x */
        xn_div(&z, x, &den);       /* x / (2 + x) */
        two_atanh_series(r, &z);
        return DEC_OK;
    }
    return xf_ln(r, &t);
}

dec_status xf_pow_pos(xnum_t *r, const xnum_t *x, const xnum_t *y)
{
    xnum_t l, t;
    dec_status st = xf_ln(&l, x);

    if (st != DEC_OK) {
        return st;
    }
    xn_mul(&t, &l, y);
    return xf_exp(r, &t);
}

/* ------------------------------------------------------------------------ */
/* trigonometric functions                                                  */
/* ------------------------------------------------------------------------ */

/* sin and cos for |r| <= ~0.8 */
static void kernel_sincos(xnum_t *s, xnum_t *c, const xnum_t *r)
{
    xnum_t r2, term, sum;
    uint32_t k;

    if (r->l[0] == 0) {
        xn_zero(s);
        xn_one(c);
        return;
    }
    xn_mul(&r2, r, r);

    term = *r;
    sum = *r;
    for (k = 1; k < 60; k++) {
        xn_mul(&term, &term, &r2);
        xn_div_u32(&term, &term, (2u * k) * (2u * k + 1u));
        xn_neg(&term, &term);
        xn_add(&sum, &sum, &term);
        if (negligible(&term, &sum)) {
            break;
        }
    }
    *s = sum;

    xn_one(&term);
    sum = term;
    for (k = 1; k < 60; k++) {
        xn_mul(&term, &term, &r2);
        xn_div_u32(&term, &term, (2u * k - 1u) * (2u * k));
        xn_neg(&term, &term);
        xn_add(&sum, &sum, &term);
        if (negligible(&term, &sum)) {
            break;
        }
    }
    *c = sum;
}

/* ---- exact radian reduction with a 126-digit pi/2 in fixed point ---- */

#define FP_N 16
typedef struct fp { uint32_t c[FP_N]; } fp_t;   /* c[0] weight B^1, c[1] weight B^0, c[j] weight B^(1-j) */

static int fp_cmp(const fp_t *a, const fp_t *b)
{
    int i;
    for (i = 0; i < FP_N; i++) {
        if (a->c[i] != b->c[i]) {
            return a->c[i] > b->c[i] ? 1 : -1;
        }
    }
    return 0;
}

/* r = a - b, requires a >= b */
static void fp_sub(fp_t *r, const fp_t *a, const fp_t *b)
{
    int64_t borrow = 0;
    int i;
    for (i = FP_N - 1; i >= 0; i--) {
        int64_t v = (int64_t)a->c[i] - (int64_t)b->c[i] - borrow;
        if (v < 0) {
            v += XN_BASE;
            borrow = 1;
        } else {
            borrow = 0;
        }
        r->c[i] = (uint32_t)v;
    }
}

/* Reduce |x| (< 10^10) modulo pi/2: |x| = k*pi/2 + r, |r| <= ~pi/4. */
static void reduce_pio2(const xnum_t *x, int64_t *kq, xnum_t *r)
{
    xnum_t ax, q;
    fp_t X, T, D;
    int64_t k;
    uint64_t carry = 0;
    int i, m, neg, jz;
    int32_t pos;

    xn_abs(&ax, x);
    xn_mul(&q, &ax, &XC_TWO_OVER_PI);
    (void)xn_to_i64(&q, XN_ROUND, &k);

    for (i = 0; i < FP_N; i++) {
        X.c[i] = 0;
        T.c[i] = 0;
    }
    for (i = 0; i < XN_LIMBS; i++) {
        pos = ax.e - 1 - i;
        if (ax.l[0] != 0 && 1 - pos >= 0 && 1 - pos < FP_N) {
            X.c[1 - pos] = ax.l[i];
        }
    }
    for (m = XC_PIO2_CHUNKS - 1; m >= 0; m--) {
        uint64_t v = (uint64_t)xc_pio2_chunks[m] * (uint64_t)k + carry;
        T.c[1 + m] = (uint32_t)(v % XN_BASE);
        carry = v / XN_BASE;
    }
    T.c[0] = (uint32_t)carry;

    neg = fp_cmp(&X, &T) < 0;
    if (neg) {
        fp_sub(&D, &T, &X);
    } else {
        fp_sub(&D, &X, &T);
    }
    jz = 0;
    while (jz < FP_N && D.c[jz] == 0) {
        jz++;
    }
    if (jz == FP_N) {
        xn_zero(r);
    } else {
        xnum_t tmp;
        xn_zero(&tmp);
        for (i = 0; i < XN_LIMBS && jz + i < FP_N; i++) {
            tmp.l[i] = D.c[jz + i];
        }
        tmp.e = 2 - jz;
        tmp.neg = (uint8_t)neg;
        if (jz + XN_LIMBS < FP_N && D.c[jz + XN_LIMBS] >= XN_BASE / 2) { /* round */
            xnum_t ulp;
            xn_zero(&ulp);
            ulp.l[0] = 1;
            ulp.e = tmp.e - XN_LIMBS + 1;
            ulp.neg = tmp.neg;
            xn_add(&tmp, &tmp, &ulp);
        }
        *r = tmp;
    }
    *kq = k;
}

dec_status xf_sincos(const xnum_t *x, int unit, xnum_t *s, xnum_t *c, int *rzero, int *quad_odd)
{
    xnum_t ax, rr, sr, cr, lim;
    int64_t quad;
    int xneg = x->neg && x->l[0] != 0;
    int zero_rem = 0;

    xn_pow10(&lim, XF_TRIG_LIMIT_EXP10);
    xn_abs(&ax, x);
    if (xn_cmp(&ax, &lim) >= 0) {
        return DEC_DOMAIN;
    }

    if (unit == XF_UNIT_RAD) {
        reduce_pio2(&ax, &quad, &rr);
        zero_rem = rr.l[0] == 0;
    } else {
        xnum_t Q, t, qq, rem, half;
        int64_t qi;

        xn_from_u64(&Q, unit == XF_UNIT_DEG ? 90u : 100u);
        xn_div(&t, &ax, &Q);
        (void)xn_to_i64(&t, XN_FLOOR, &qi);
        xn_from_i64(&qq, qi);
        xn_mul(&t, &qq, &Q);
        xn_sub(&rem, &ax, &t);
        if (rem.neg && rem.l[0] != 0) {
            xn_add(&rem, &rem, &Q);
            qi--;
        } else if (xn_cmp(&rem, &Q) >= 0) {
            xn_sub(&rem, &rem, &Q);
            qi++;
        }
        xn_div_u32(&half, &Q, 2);
        if (xn_cmp(&rem, &half) > 0) {
            xn_sub(&rem, &rem, &Q);
            qi++;
        }
        quad = qi;
        zero_rem = rem.l[0] == 0;
        xn_mul(&rr, &rem, unit == XF_UNIT_DEG ? &XC_RAD_PER_DEG : &XC_RAD_PER_GRAD);
    }

    if (zero_rem) {
        xn_zero(&sr);
        xn_one(&cr);
    } else {
        kernel_sincos(&sr, &cr, &rr);
    }

    switch ((int)(quad & 3)) {
    case 0: *s = sr;  *c = cr;  break;
    case 1: *s = cr;  xn_neg(c, &sr); break;
    case 2: xn_neg(s, &sr); xn_neg(c, &cr); break;
    default: xn_neg(s, &cr); *c = sr; break;
    }
    if (xneg) {
        xn_neg(s, s);   /* sin odd, cos even */
    }
    *rzero = zero_rem;
    *quad_odd = (int)(quad & 1);
    return DEC_OK;
}

/* ---- inverse trigonometric ---- */

static void xn_sqrt_nonneg(xnum_t *r, const xnum_t *a)
{
    if (a->l[0] == 0 || a->neg) {
        xn_zero(r);
    } else {
        xn_sqrt(r, a);
    }
}

dec_status xf_atan(xnum_t *r, const xnum_t *x)
{
    xnum_t ax, t, u, one, z2, term, sum, tt, res;
    int neg = x->neg;
    int inverted = 0;
    int halvings = 0;
    uint32_t n;
    int i;

    if (x->l[0] == 0) {
        xn_zero(r);
        return DEC_OK;
    }
    xn_abs(&ax, x);
    xn_one(&one);
    if (xn_cmp(&ax, &one) > 0) {
        xn_div(&ax, &one, &ax);
        inverted = 1;
    }
    xn_from_u64(&t, 1);
    xn_div_u32(&t, &t, 10);       /* 0.1 */
    while (xn_cmp(&ax, &t) > 0 && halvings < 3) {
        xn_mul(&u, &ax, &ax);
        xn_add(&u, &u, &one);
        xn_sqrt(&u, &u);
        xn_add(&u, &u, &one);
        xn_div(&ax, &ax, &u);
        halvings++;
    }
    xn_mul(&z2, &ax, &ax);
    term = ax;
    sum = ax;
    for (n = 1; n < 80; n++) {
        xn_mul(&term, &term, &z2);
        xn_neg(&term, &term);
        xn_div_u32(&tt, &term, 2u * n + 1u);
        xn_add(&sum, &sum, &tt);
        if (negligible(&tt, &sum)) {
            break;
        }
    }
    for (i = 0; i < halvings; i++) {
        xn_mul_u32(&sum, &sum, 2);
    }
    res = sum;
    if (inverted) {
        xn_sub(&res, &XC_PIO2, &res);
    }
    if (neg) {
        xn_neg(&res, &res);
    }
    *r = res;
    return DEC_OK;
}

dec_status xf_atan2(xnum_t *r, const xnum_t *y, const xnum_t *x)
{
    xnum_t q, a;

    if (x->l[0] == 0 && y->l[0] == 0) {
        return DEC_DOMAIN;
    }
    if (x->l[0] == 0) {
        *r = XC_PIO2;
        r->neg = y->neg;
        return DEC_OK;
    }
    if (y->l[0] == 0) {
        if (x->neg) {
            *r = XC_PI;
        } else {
            xn_zero(r);
        }
        return DEC_OK;
    }
    xn_div(&q, y, x);
    (void)xf_atan(&a, &q);
    if (!x->neg) {
        *r = a;
    } else if (!y->neg) {
        xn_add(r, &a, &XC_PI);
    } else {
        xn_sub(r, &a, &XC_PI);
    }
    return DEC_OK;
}

dec_status xf_asin(xnum_t *r, const xnum_t *x)
{
    xnum_t one, a, b, c, d;

    xn_one(&one);
    xn_abs(&a, x);
    if (xn_cmp(&a, &one) > 0) {
        return DEC_DOMAIN;
    }
    if (xn_cmp(&a, &one) == 0) {
        *r = XC_PIO2;
        r->neg = x->neg;
        return DEC_OK;
    }
    xn_sub(&b, &one, &a);
    xn_add(&c, &one, &a);
    xn_mul(&b, &b, &c);            /* 1 - x^2 */
    xn_sqrt_nonneg(&d, &b);
    return xf_atan2(r, x, &d);
}

dec_status xf_acos(xnum_t *r, const xnum_t *x)
{
    xnum_t one, a, b, c, d;

    xn_one(&one);
    xn_abs(&a, x);
    if (xn_cmp(&a, &one) > 0) {
        return DEC_DOMAIN;
    }
    xn_sub(&b, &one, x);
    xn_add(&c, &one, x);
    xn_mul(&b, &b, &c);
    xn_sqrt_nonneg(&d, &b);        /* sqrt(1 - x^2) */
    if (d.l[0] == 0) {
        if (x->neg) {
            *r = XC_PI;
        } else {
            xn_zero(r);
        }
        return DEC_OK;
    }
    return xf_atan2(r, &d, x);
}

/* ------------------------------------------------------------------------ */
/* hyperbolic                                                               */
/* ------------------------------------------------------------------------ */

static int small_arg(const xnum_t *x)
{
    xnum_t h;
    xn_from_u64(&h, 1);
    xn_div_u32(&h, &h, 2);
    return xn_cmp_abs(x, &h) < 0;
}

dec_status xf_sinh(xnum_t *r, const xnum_t *x)
{
    xnum_t ex, enx, one, t, x2, term, sum;
    uint32_t k;

    if (x->l[0] == 0) {
        xn_zero(r);
        return DEC_OK;
    }
    if (small_arg(x)) {
        xn_mul(&x2, x, x);
        term = *x;
        sum = *x;
        for (k = 1; k < 60; k++) {
            xn_mul(&term, &term, &x2);
            xn_div_u32(&term, &term, (2u * k) * (2u * k + 1u));
            xn_add(&sum, &sum, &term);
            if (negligible(&term, &sum)) {
                break;
            }
        }
        *r = sum;
        return DEC_OK;
    }
    xf_exp(&ex, x);
    xn_one(&one);
    xn_div(&enx, &one, &ex);
    xn_sub(&t, &ex, &enx);
    xn_div_u32(r, &t, 2);
    return DEC_OK;
}

dec_status xf_cosh(xnum_t *r, const xnum_t *x)
{
    xnum_t ex, enx, one, t;

    xf_exp(&ex, x);
    xn_one(&one);
    xn_div(&enx, &one, &ex);
    xn_add(&t, &ex, &enx);
    xn_div_u32(r, &t, 2);
    return DEC_OK;
}

dec_status xf_tanh(xnum_t *r, const xnum_t *x)
{
    xnum_t s, c, lim;

    if (x->l[0] == 0) {
        xn_zero(r);
        return DEC_OK;
    }
    xn_from_u64(&lim, 40);
    if (xn_cmp_abs(x, &lim) > 0) {
        xn_one(r);
        r->neg = x->neg;
        return DEC_OK;
    }
    xf_sinh(&s, x);
    xf_cosh(&c, x);
    xn_div(r, &s, &c);
    return DEC_OK;
}

dec_status xf_asinh(xnum_t *r, const xnum_t *x)
{
    xnum_t ax, one, u, v, w;
    int neg = x->neg;
    dec_status st;

    if (x->l[0] == 0) {
        xn_zero(r);
        return DEC_OK;
    }
    xn_abs(&ax, x);
    xn_one(&one);
    xn_mul(&u, &ax, &ax);
    xn_add(&u, &u, &one);
    xn_sqrt(&v, &u);               /* sqrt(1 + x^2) */
    xn_add(&w, &v, &one);
    xn_div(&u, &ax, &w);           /* x / (1 + sqrt(1+x^2)) * x  below */
    xn_mul(&u, &u, &ax);           /* x^2 / (1 + sqrt(1+x^2)) = sqrt(1+x^2) - 1 */
    xn_add(&u, &u, &ax);           /* x + sqrt(1+x^2) - 1 */
    st = xf_log1p(r, &u);
    if (neg) {
        xn_neg(r, r);
    }
    return st;
}

dec_status xf_acosh(xnum_t *r, const xnum_t *x)
{
    xnum_t one, t, u, v;

    xn_one(&one);
    if (xn_cmp(x, &one) < 0) {
        return DEC_DOMAIN;
    }
    xn_sub(&t, x, &one);
    if (t.l[0] == 0) {
        xn_zero(r);
        return DEC_OK;
    }
    xn_from_u64(&u, 2);
    xn_add(&u, &t, &u);
    xn_mul(&u, &u, &t);            /* t (t + 2) = x^2 - 1 */
    xn_sqrt(&v, &u);
    xn_add(&v, &v, &t);
    return xf_log1p(r, &v);
}

dec_status xf_atanh(xnum_t *r, const xnum_t *x)
{
    xnum_t ax, one, num, den, z, t;
    dec_status st;

    xn_one(&one);
    xn_abs(&ax, x);
    if (xn_cmp(&ax, &one) >= 0) {
        return DEC_DOMAIN;
    }
    if (x->l[0] == 0) {
        xn_zero(r);
        return DEC_OK;
    }
    xn_mul_u32(&num, x, 2);
    xn_sub(&den, &one, x);
    xn_div(&z, &num, &den);        /* 2x / (1 - x) */
    st = xf_log1p(&t, &z);
    if (st != DEC_OK) {
        return st;
    }
    xn_div_u32(r, &t, 2);
    return DEC_OK;
}

void xf_rad_to_unit(xnum_t *r, const xnum_t *rad, int unit)
{
    if (unit == XF_UNIT_DEG) {
        xn_mul(r, rad, &XC_DEG_PER_RAD);
    } else if (unit == XF_UNIT_GRAD) {
        xn_mul(r, rad, &XC_GRAD_PER_RAD);
    } else {
        *r = *rad;
    }
}
