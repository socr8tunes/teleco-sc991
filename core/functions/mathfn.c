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
/* xnum-core implementations                                                */
/* ------------------------------------------------------------------------ */

static int is_pow10_xn(const xnum_t *x)
{
    int i;
    uint32_t l = x->l[0];

    for (i = 1; i < XN_LIMBS; i++) {
        if (x->l[i] != 0) {
            return 0;
        }
    }
    while (l >= 10 && l % 10 == 0) {
        l /= 10;
    }
    return l == 1;
}

/* integer value of an xnum if it is an integer of modest size */
static int xn_int(const xnum_t *x, int64_t *n)
{
    return xn_is_int(x) && xn_to_i64(x, XN_TRUNC, n) == 0;
}

dec_status fx_exp(dec_t *r, const xnum_t *x)
{
    xnum_t y;
    return done(r, &y, xf_exp(&y, x));
}

dec_status fx_exp10(dec_t *r, const xnum_t *x)
{
    xnum_t a, y;
    int64_t k;

    if (xn_int(x, &k) && k > -1000 && k < 1000) {
        xn_pow10(&y, (int32_t)k);
        return done(r, &y, DEC_OK);
    }
    xn_mul(&a, x, &XC_LN10);
    return done(r, &y, xf_exp(&y, &a));
}

dec_status fx_ln(dec_t *r, const xnum_t *x)
{
    xnum_t y;
    return done(r, &y, xf_ln(&y, x));
}

dec_status fx_log10(dec_t *r, const xnum_t *x)
{
    xnum_t y, t;
    dec_status st;

    if (x->l[0] == 0 || x->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    if (is_pow10_xn(x)) {
        return dec_from_i64(r, xn_dec_exponent(x));
    }
    st = xf_ln(&t, x);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_mul(&y, &t, &XC_INV_LN10);
    return xn_to_dec(r, &y);
}

dec_status fx_log(dec_t *r, const xnum_t *base, const xnum_t *x)
{
    xnum_t la, lb, y, one;
    dec_status st;

    xn_from_u64(&one, 1);
    if (base->l[0] == 0 || base->neg || xn_cmp(base, &one) == 0 || x->l[0] == 0 || x->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    st = xf_ln(&la, x);
    if (st == DEC_OK) {
        st = xf_ln(&lb, base);
    }
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_div(&y, &la, &lb);
    return xn_to_dec(r, &y);
}

dec_status fx_pow(dec_t *r, const xnum_t *x, const xnum_t *y)
{
    xnum_t z, ax;
    int64_t n;

    if (y->l[0] == 0) {
        if (x->l[0] == 0) {
            dec_zero(r);
            return DEC_DOMAIN;          /* 0^0 */
        }
        return dec_from_i64(r, 1);
    }
    if (x->l[0] == 0) {
        dec_zero(r);
        return y->neg ? DEC_DIV_BY_ZERO : DEC_OK;
    }
    if (xn_int(y, &n)) {
        if (n > 2000000000LL || n < -2000000000LL) {
            xn_zero(&z);
            z.l[0] = 1;
            z.e = ((xn_dec_exponent(x) < 0) == (n > 0)) ? -XN_EMAX : XN_EMAX;
            return done(r, &z, DEC_OK);
        }
        xn_powi(&z, x, n);
        return done(r, &z, DEC_OK);
    }
    if (x->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    ax = *x;
    return done(r, &z, xf_pow_pos(&z, &ax, y));
}

dec_status fx_cbrt(dec_t *r, const xnum_t *x)
{
    xnum_t a, l, z;
    dec_status st;

    if (x->l[0] == 0) {
        dec_zero(r);
        return DEC_OK;
    }
    a = *x;
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

dec_status fx_xroot(dec_t *r, const xnum_t *n, const xnum_t *x)
{
    xnum_t a, l, z;
    int64_t k = 0;
    int is_int;
    dec_status st;

    if (n->l[0] == 0) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    if (x->l[0] == 0) {
        dec_zero(r);
        return n->neg ? DEC_DIV_BY_ZERO : DEC_OK;
    }
    is_int = xn_int(n, &k);
    if (x->neg && !(is_int && (k & 1))) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    a = *x;
    a.neg = 0;
    st = xf_ln(&l, &a);
    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xn_div(&l, &l, n);             /* ln(x) / n */
    xf_exp(&z, &l);
    z.neg = x->neg;
    return xn_to_dec(r, &z);
}

/* ---- trigonometry ---- */

dec_status fx_sin(dec_t *r, const xnum_t *x, ang_t a)
{
    xnum_t s, c;
    int rz, qo;
    dec_status st = xf_sincos(x, unit_of(a), &s, &c, &rz, &qo);
    return done(r, &s, st);
}

dec_status fx_cos(dec_t *r, const xnum_t *x, ang_t a)
{
    xnum_t s, c;
    int rz, qo;
    dec_status st = xf_sincos(x, unit_of(a), &s, &c, &rz, &qo);
    return done(r, &c, st);
}

dec_status fx_tan(dec_t *r, const xnum_t *x, ang_t a)
{
    xnum_t s, c, t;
    int rz, qo;
    dec_status st = xf_sincos(x, unit_of(a), &s, &c, &rz, &qo);

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

static dec_status inv_finish(dec_t *r, const xnum_t *y, dec_status st, ang_t a)
{
    xnum_t u;

    if (st != DEC_OK) {
        dec_zero(r);
        return st;
    }
    xf_rad_to_unit(&u, y, unit_of(a));
    return xn_to_dec(r, &u);
}

dec_status fx_asin(dec_t *r, const xnum_t *x, ang_t a)
{
    xnum_t y;
    dec_status st = xf_asin(&y, x);
    return inv_finish(r, &y, st, a);
}

dec_status fx_acos(dec_t *r, const xnum_t *x, ang_t a)
{
    xnum_t y;
    dec_status st = xf_acos(&y, x);
    return inv_finish(r, &y, st, a);
}

dec_status fx_atan(dec_t *r, const xnum_t *x, ang_t a)
{
    xnum_t y;
    dec_status st = xf_atan(&y, x);
    return inv_finish(r, &y, st, a);
}

dec_status fx_atan2(dec_t *r, const xnum_t *y, const xnum_t *x, ang_t a)
{
    xnum_t t;
    dec_status st = xf_atan2(&t, y, x);
    return inv_finish(r, &t, st, a);
}

#define FX_HYP(name, kernel)                                   \
    dec_status name(dec_t *r, const xnum_t *x)                 \
    {                                                          \
        xnum_t y;                                              \
        return done(r, &y, kernel(&y, x));                     \
    }

FX_HYP(fx_sinh, xf_sinh)
FX_HYP(fx_cosh, xf_cosh)
FX_HYP(fx_tanh, xf_tanh)
FX_HYP(fx_asinh, xf_asinh)
FX_HYP(fx_acosh, xf_acosh)
FX_HYP(fx_atanh, xf_atanh)

/* ---- decimal wrappers ---- */

#define FN1(name, fx)                                          \
    dec_status name(dec_t *r, const dec_t *x)                  \
    {                                                          \
        xnum_t a;                                              \
        xn_from_dec(&a, x);                                    \
        return fx(r, &a);                                      \
    }

#define FN2(name, fx)                                          \
    dec_status name(dec_t *r, const dec_t *x, const dec_t *y)  \
    {                                                          \
        xnum_t a, b;                                           \
        xn_from_dec(&a, x);                                    \
        xn_from_dec(&b, y);                                    \
        return fx(r, &a, &b);                                  \
    }

#define FNT(name, fx)                                          \
    dec_status name(dec_t *r, const dec_t *x, ang_t m)         \
    {                                                          \
        xnum_t a;                                              \
        xn_from_dec(&a, x);                                    \
        return fx(r, &a, m);                                   \
    }

FN1(fn_exp, fx_exp)
FN1(fn_exp10, fx_exp10)
FN1(fn_ln, fx_ln)
FN1(fn_log10, fx_log10)
FN1(fn_cbrt, fx_cbrt)
FN2(fn_log, fx_log)
FN2(fn_pow, fx_pow)
FN2(fn_xroot, fx_xroot)
FNT(fn_sin, fx_sin)
FNT(fn_cos, fx_cos)
FNT(fn_tan, fx_tan)
FNT(fn_asin, fx_asin)
FNT(fn_acos, fx_acos)
FNT(fn_atan, fx_atan)
FN1(fn_sinh, fx_sinh)
FN1(fn_cosh, fx_cosh)
FN1(fn_tanh, fx_tanh)
FN1(fn_asinh, fx_asinh)
FN1(fn_acosh, fx_acosh)
FN1(fn_atanh, fx_atanh)

dec_status fn_atan2(dec_t *r, const dec_t *y, const dec_t *x, ang_t a)
{
    xnum_t vy, vx;

    xn_from_dec(&vy, y);
    xn_from_dec(&vx, x);
    return fx_atan2(r, &vy, &vx, a);
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
