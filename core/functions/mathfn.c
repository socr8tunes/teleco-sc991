#include "functions/mathfn.h"
#include "functions/xfunc.h"
#include "numeric/xconst.h"
#include "numeric/wide.h"

/* ------------------------------------------------------------------------ */
/* helpers                                                                  */
/* ------------------------------------------------------------------------ */

static dec_status done(dec_t *r, const xnum_t *x, dec_status st)
{
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    return xn_to_dec(r, x);
}

static int unit_of(ang_t a)
{
    return a == ANG_DEG ? XF_UNIT_DEG : (a == ANG_GRAD ? XF_UNIT_GRAD : XF_UNIT_RAD);
}

int dec_to_i64(const dec_t *x, int64_t *out)
{
    uint64_t v;

    if (x->m == 0) {
        *out = 0;
        return 0;
    }
    if (x->exp < 0 || x->exp > 18) {
        return -1;
    }
    if (x->exp < 15) {
        uint64_t p = u64_pow10[15 - x->exp];
        if (x->m % p != 0) {
            return -1;
        }
        v = x->m / p;
    } else {
        v = x->m * u64_pow10[x->exp - 15];
        if (x->exp == 18 && v / 1000 != x->m) {
            return -1;
        }
    }
    if (v > (uint64_t)INT64_MAX) {
        return -1;
    }
    *out = x->neg ? -(int64_t)v : (int64_t)v;
    return 0;
}

/* Truncate toward zero; *had_frac set if anything was dropped. */
static void trunc_core(dec_t *r, const dec_t *x, int *had_frac)
{
    *had_frac = 0;
    if (x->m == 0 || x->exp >= DEC_DIGITS - 1) {
        *r = *x;
        return;
    }
    if (x->exp < 0) {
        *had_frac = 1;
        dec_zero(r);
        return;
    }
    {
        uint64_t p = u64_pow10[(DEC_DIGITS - 1) - x->exp];
        uint64_t ip = x->m / p;
        *had_frac = (x->m % p) != 0;
        (void)dec_from_i64(r, x->neg ? -(int64_t)ip : (int64_t)ip);
    }
}

dec_status fn_trunc(dec_t *r, const dec_t *x)
{
    int f;
    trunc_core(r, x, &f);
    return DEC_OK;
}

dec_status fn_floor(dec_t *r, const dec_t *x)
{
    int f;
    dec_t one;

    trunc_core(r, x, &f);
    if (f && x->neg) {
        dec_from_i64(&one, 1);
        return dec_sub(r, r, &one);
    }
    return DEC_OK;
}

dec_status fn_ceil(dec_t *r, const dec_t *x)
{
    int f;
    dec_t one;

    trunc_core(r, x, &f);
    if (f && !x->neg) {
        dec_from_i64(&one, 1);
        return dec_add(r, r, &one);
    }
    return DEC_OK;
}

dec_status fn_frac(dec_t *r, const dec_t *x)
{
    dec_t t;
    int f;

    trunc_core(&t, x, &f);
    return dec_sub(r, x, &t);
}

void fn_pi(dec_t *r)
{
    (void)xn_to_dec(r, &XC_PI);
}

void fn_e(dec_t *r)
{
    (void)xn_to_dec(r, &XC_E);
}

/* ------------------------------------------------------------------------ */
/* exp / log / pow                                                          */
/* ------------------------------------------------------------------------ */

dec_status fn_exp(dec_t *r, const dec_t *x)
{
    xnum_t a, y;
    xn_from_dec(&a, x);
    return done(r, &y, xf_exp(&y, &a));
}

dec_status fn_exp10(dec_t *r, const dec_t *x)
{
    xnum_t a, y;
    int64_t k;

    xn_from_dec(&a, x);
    if (dec_to_i64(x, &k) == 0 && k > -1000 && k < 1000) {
        xn_pow10(&y, (int32_t)k);
        return done(r, &y, DEC_OK);
    }
    xn_mul(&a, &a, &XC_LN10);
    return done(r, &y, xf_exp(&y, &a));
}

dec_status fn_ln(dec_t *r, const dec_t *x)
{
    xnum_t a, y;
    xn_from_dec(&a, x);
    return done(r, &y, xf_ln(&y, &a));
}

dec_status fn_log10(dec_t *r, const dec_t *x)
{
    xnum_t a, y, t;
    dec_status st;

    if (x->m == 0 || x->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    if (x->m == DEC_MANT_MIN) {  /* exact power of ten */
        return dec_from_i64(r, x->exp);
    }
    xn_from_dec(&a, x);
    st = xf_ln(&t, &a);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_mul(&y, &t, &XC_INV_LN10);
    return xn_to_dec(r, &y);
}

dec_status fn_log(dec_t *r, const dec_t *base, const dec_t *x)
{
    xnum_t a, b, la, lb, y;
    dec_status st;
    dec_t one;

    dec_from_i64(&one, 1);
    if (base->m == 0 || base->neg || dec_cmp(base, &one) == 0 || x->m == 0 || x->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    xn_from_dec(&a, x);
    xn_from_dec(&b, base);
    st = xf_ln(&la, &a);
    if (st == DEC_OK) {
        st = xf_ln(&lb, &b);
    }
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_div(&y, &la, &lb);
    return xn_to_dec(r, &y);
}

dec_status fn_pow(dec_t *r, const dec_t *x, const dec_t *y)
{
    xnum_t a, b, z;
    int64_t n;

    if (y->m == 0) {
        if (x->m == 0) {
            dec_zero(r);
            return DEC_DOMAIN;          /* 0^0 */
        }
        return dec_from_i64(r, 1);
    }
    if (x->m == 0) {
        dec_zero(r);
        return y->neg ? DEC_DIV_BY_ZERO : DEC_OK;
    }
    xn_from_dec(&a, x);
    if (dec_to_i64(y, &n) == 0) {
        if (n > 2000000000LL || n < -2000000000LL) {
            xn_zero(&z);
            z.l[0] = 1;
            z.e = ((xn_dec_exponent(&a) < 0) == (n > 0)) ? -XN_EMAX : XN_EMAX;
            return done(r, &z, DEC_OK);
        }
        xn_powi(&z, &a, n);
        return done(r, &z, DEC_OK);
    }
    if (x->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    xn_from_dec(&b, y);
    return done(r, &z, xf_pow_pos(&z, &a, &b));
}

dec_status fn_cbrt(dec_t *r, const dec_t *x)
{
    xnum_t a, l, z;
    dec_status st;

    if (x->m == 0) {
        dec_zero(r);
        return DEC_OK;
    }
    xn_from_dec(&a, x);
    a.neg = 0;
    st = xf_ln(&l, &a);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_div_u32(&l, &l, 3);
    xf_exp(&z, &l);
    z.neg = x->neg;
    return xn_to_dec(r, &z);
}

dec_status fn_xroot(dec_t *r, const dec_t *n, const dec_t *x)
{
    xnum_t a, e, one, l, z;
    int64_t k = 0;
    int is_int;
    dec_status st;

    if (n->m == 0) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    if (x->m == 0) {
        dec_zero(r);
        return n->neg ? DEC_DIV_BY_ZERO : DEC_OK;
    }
    is_int = dec_to_i64(n, &k) == 0;
    if (x->neg && !(is_int && (k & 1))) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    xn_from_dec(&a, x);
    a.neg = 0;
    xn_from_dec(&e, n);
    xn_from_u64(&one, 1);
    st = xf_ln(&l, &a);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_div(&l, &l, &e);            /* ln(x) / n */
    xf_exp(&z, &l);
    z.neg = x->neg;
    return xn_to_dec(r, &z);
}

/* ------------------------------------------------------------------------ */
/* trigonometry                                                             */
/* ------------------------------------------------------------------------ */

dec_status fn_sin(dec_t *r, const dec_t *x, ang_t a)
{
    xnum_t v, s, c;
    int rz, qo;
    dec_status st;

    xn_from_dec(&v, x);
    st = xf_sincos(&v, unit_of(a), &s, &c, &rz, &qo);
    return done(r, &s, st);
}

dec_status fn_cos(dec_t *r, const dec_t *x, ang_t a)
{
    xnum_t v, s, c;
    int rz, qo;
    dec_status st;

    xn_from_dec(&v, x);
    st = xf_sincos(&v, unit_of(a), &s, &c, &rz, &qo);
    return done(r, &c, st);
}

dec_status fn_tan(dec_t *r, const dec_t *x, ang_t a)
{
    xnum_t v, s, c, t;
    int rz, qo;
    dec_status st;

    xn_from_dec(&v, x);
    st = xf_sincos(&v, unit_of(a), &s, &c, &rz, &qo);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    if (xn_is_zero(&c)) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    xn_div(&t, &s, &c);
    return xn_to_dec(r, &t);
}

dec_status fn_asin(dec_t *r, const dec_t *x, ang_t a)
{
    xnum_t v, y, u;
    dec_status st;

    xn_from_dec(&v, x);
    st = xf_asin(&y, &v);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xf_rad_to_unit(&u, &y, unit_of(a));
    return xn_to_dec(r, &u);
}

dec_status fn_acos(dec_t *r, const dec_t *x, ang_t a)
{
    xnum_t v, y, u;
    dec_status st;

    xn_from_dec(&v, x);
    st = xf_acos(&y, &v);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xf_rad_to_unit(&u, &y, unit_of(a));
    return xn_to_dec(r, &u);
}

dec_status fn_atan(dec_t *r, const dec_t *x, ang_t a)
{
    xnum_t v, y, u;

    xn_from_dec(&v, x);
    (void)xf_atan(&y, &v);
    xf_rad_to_unit(&u, &y, unit_of(a));
    return xn_to_dec(r, &u);
}

dec_status fn_atan2(dec_t *r, const dec_t *y, const dec_t *x, ang_t a)
{
    xnum_t vy, vx, t, u;
    dec_status st;

    xn_from_dec(&vy, y);
    xn_from_dec(&vx, x);
    st = xf_atan2(&t, &vy, &vx);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xf_rad_to_unit(&u, &t, unit_of(a));
    return xn_to_dec(r, &u);
}

dec_status fn_convert_angle(dec_t *r, const dec_t *x, ang_t from, ang_t to)
{
    xnum_t v, t, u;

    if (from == to || x->m == 0) {
        *r = *x;
        return DEC_OK;
    }
    xn_from_dec(&v, x);
    /* to radians */
    if (from == ANG_DEG) {
        xn_mul(&t, &v, &XC_RAD_PER_DEG);
    } else if (from == ANG_GRAD) {
        xn_mul(&t, &v, &XC_RAD_PER_GRAD);
    } else {
        t = v;
    }
    xf_rad_to_unit(&u, &t, unit_of(to));
    return xn_to_dec(r, &u);
}

/* ------------------------------------------------------------------------ */
/* hyperbolic                                                               */
/* ------------------------------------------------------------------------ */

#define HYP(name, kernel)                                      \
    dec_status name(dec_t *r, const dec_t *x)                  \
    {                                                          \
        xnum_t v, y;                                           \
        xn_from_dec(&v, x);                                    \
        return done(r, &y, kernel(&y, &v));                    \
    }

HYP(fn_sinh, xf_sinh)
HYP(fn_cosh, xf_cosh)
HYP(fn_tanh, xf_tanh)
HYP(fn_asinh, xf_asinh)
HYP(fn_acosh, xf_acosh)
HYP(fn_atanh, xf_atanh)

/* ------------------------------------------------------------------------ */
/* combinatorics                                                            */
/* ------------------------------------------------------------------------ */

dec_status fn_fact(dec_t *r, const dec_t *x)
{
    int64_t n;
    xnum_t acc, f;
    int64_t i;

    if (dec_to_i64(x, &n) != 0 || n < 0) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    if (n > 69) {
        dec_zero(r);
        return DEC_OVERFLOW;    /* 70! >= 1e100 */
    }
    xn_from_u64(&acc, 1);
    for (i = 2; i <= n; i++) {
        xn_from_i64(&f, i);
        xn_mul(&acc, &acc, &f);
    }
    return xn_to_dec(r, &acc);
}

/* common argument check: n >= k >= 0 integers, n < 1e10 */
static int nk_args(const dec_t *n, const dec_t *k, int64_t *pn, int64_t *pk)
{
    if (dec_to_i64(n, pn) != 0 || dec_to_i64(k, pk) != 0) {
        return -1;
    }
    if (*pn < 0 || *pk < 0 || *pk > *pn || *pn >= 10000000000LL) {
        return -1;
    }
    return 0;
}

dec_status fn_npr(dec_t *r, const dec_t *n, const dec_t *k)
{
    int64_t pn, pk, i;
    xnum_t acc, f;

    if (nk_args(n, k, &pn, &pk) != 0) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    xn_from_u64(&acc, 1);
    for (i = 0; i < pk; i++) {
        xn_from_i64(&f, pn - i);
        xn_mul(&acc, &acc, &f);
        if (acc.e > 12) {
            dec_zero(r);
            return DEC_OVERFLOW;
        }
    }
    return xn_to_dec(r, &acc);
}

dec_status fn_ncr(dec_t *r, const dec_t *n, const dec_t *k)
{
    int64_t pn, pk, m, i;
    xnum_t acc, f;

    if (nk_args(n, k, &pn, &pk) != 0) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    m = pk < pn - pk ? pk : pn - pk;
    xn_from_u64(&acc, 1);
    for (i = 1; i <= m; i++) {
        xn_from_i64(&f, pn - m + i);
        xn_mul(&acc, &acc, &f);
        xn_div_u32(&acc, &acc, (uint32_t)i);
        if (acc.e > 12) {
            dec_zero(r);
            return DEC_OVERFLOW;
        }
    }
    return xn_to_dec(r, &acc);
}
