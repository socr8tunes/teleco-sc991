#include "functions/dist.h"
#include "functions/xfunc.h"
#include "numeric/xconst.h"

#define CHK(x) do { dec_status _s = (x); if (DEC_STATUS_IS_ERROR(_s)) return _s; } while (0)

/* upper tail R(|t|), full precision */
static dec_status tail_of(xnum_t *tail, const xnum_t *at)
{
    xnum_t two_pi, sq2pi, half, tmp, q, u, s, eps, k, term, lim;
    int kk;

    xn_mul_u32(&two_pi, &XC_PI, 2);
    xn_sqrt(&sq2pi, &two_pi);
    xn_from_i64(&half, 1);
    xn_div_u32(&half, &half, 2);
    xn_from_i64(&lim, 3);

    if (xn_cmp_abs(at, &lim) <= 0) {
        xn_mul(&q, at, at);
        xn_div_u32(&q, &q, 2);
        xn_from_i64(&u, 1);
        xn_from_i64(&s, 1);
        xn_pow10(&eps, -52);
        for (kk = 1; kk < 2000; kk++) {
            xn_mul(&u, &u, &q);
            xn_div_u32(&u, &u, (uint32_t)kk);
            u.neg = (uint8_t)!u.neg;
            xn_div_u32(&term, &u, (uint32_t)(2 * kk + 1));
            xn_add(&s, &s, &term);
            xn_from_i64(&k, kk);
            if (xn_cmp_abs(&u, &eps) < 0 && xn_cmp(&k, &q) > 0) break;
        }
        xn_mul(&tmp, at, &s);
        xn_div(&tmp, &tmp, &sq2pi);
        xn_sub(tail, &half, &tmp);
        return DEC_OK;
    }
    {
        xnum_t b, kx, e, ph;
        xn_mul(&q, at, at);
        xn_div_u32(&q, &q, 2);
        xn_from_i64(&lim, 700);
        if (xn_cmp(&q, &lim) > 0) { xn_zero(tail); return DEC_OK; }
        b = *at;
        for (kk = 300; kk >= 1; kk--) {
            xn_from_i64(&kx, kk);
            xn_div(&kx, &kx, &b);
            xn_add(&b, at, &kx);
        }
        xn_neg(&q, &q);
        xf_exp(&e, &q);
        xn_div(&ph, &e, &sq2pi);
        xn_div(tail, &ph, &b);
        return DEC_OK;
    }
}

dec_status dist_norm(int which, dec_t *r, const dec_t *t)
{
    xnum_t x, at, tail, one, half, res;
    int neg;

    xn_from_dec(&x, t);
    if (xn_is_zero(&x)) {
        if (which == NORM_Q) { dec_zero(r); return DEC_OK; }
        return dec_from_str(r, "0.5");
    }
    neg = x.neg;
    at = x;
    at.neg = 0;
    CHK(tail_of(&tail, &at));
    xn_from_i64(&one, 1);
    xn_from_i64(&half, 1);
    xn_div_u32(&half, &half, 2);
    switch (which) {
    case NORM_P: if (neg) res = tail; else xn_sub(&res, &one, &tail); break;
    case NORM_R: if (neg) xn_sub(&res, &one, &tail); else res = tail; break;
    default:
        xn_sub(&res, &half, &tail);       /* area from 0 to |t| */
        res.neg = (uint8_t)(neg && !xn_is_zero(&res));
        break;
    }
    if (xn_is_zero(&res)) { dec_zero(r); return DEC_OK; }
    return xn_to_dec(r, &res);
}

/* ======================================================================== */
/* Discrete distributions and the inverse normal                            */
/* ======================================================================== */

static void xn_small(xnum_t *r, int64_t v) { xn_from_i64(r, v); }

/* ln(n!) */
static dec_status ln_fact(xnum_t *r, int64_t n)
{
    xnum_t prod, f, z, lnz, t, z2, zp, term, c;
    int64_t i;
    if (n < 2) { xn_zero(r); return DEC_OK; }
    if (n <= 25) {
        xn_small(&prod, 1);
        for (i = 2; i <= n; i++) xn_mul_u32(&prod, &prod, (uint32_t)i);
        return xf_ln(r, &prod);
    }
    /* Stirling series for ln Gamma(z), z = n + 1 >= 26 */
    xn_small(&z, n + 1);
    CHK(xf_ln(&lnz, &z));
    xn_small(&f, 1); xn_div_u32(&f, &f, 2);
    xn_sub(&t, &z, &f);                    /* z - 1/2 */
    xn_mul(r, &t, &lnz);
    xn_sub(r, r, &z);
    xn_mul_u32(&t, &XC_PI, 2);
    CHK(xf_ln(&t, &t));
    xn_div_u32(&t, &t, 2);                 /* 1/2 ln(2 pi) */
    xn_add(r, r, &t);
    /* + 1/(12z) - 1/(360 z^3) + 1/(1260 z^5) - 1/(1680 z^7) + 1/(1188 z^9) - 691/(360360 z^11) + 1/(156 z^13) */
    {
        static const struct { int sign; uint32_t num, den; } K[] = {
            {1, 1, 12}, {-1, 1, 360}, {1, 1, 1260}, {-1, 1, 1680}, {1, 1, 1188}, {-1, 691, 360360}, {1, 1, 156},
        };
        int k;
        xn_small(&c, 1);
        xn_div(&zp, &c, &z);               /* 1/z */
        xn_mul(&z2, &zp, &zp);             /* 1/z^2 */
        for (k = 0; k < 7; k++) {
            xn_mul_u32(&term, &zp, K[k].num);
            xn_div_u32(&term, &term, K[k].den);
            if (K[k].sign < 0) xn_neg(&term, &term);
            xn_add(r, r, &term);
            xn_mul(&zp, &zp, &z2);
        }
    }
    return DEC_OK;
}

static dec_status finish(dec_t *r, const xnum_t *x)
{
    if (xn_is_zero(x)) { dec_zero(r); return DEC_OK; }
    if (xn_sign(x) < 0) { dec_zero(r); return DEC_OK; }
    return xn_to_dec(r, x);
}

/* negligible: t < s * 1e-30 */
static int negligible(const xnum_t *t, const xnum_t *s)
{
    xnum_t e, lim;
    xn_pow10(&e, -30);
    xn_mul(&lim, s, &e);
    return xn_cmp(t, &lim) < 0;
}

/* ---- binomial ---- */
static int check_binom(int64_t x, int64_t n, const dec_t *p, xnum_t *px)
{
    dec_t zero, one;
    dec_zero(&zero); dec_from_i64(&one, 1);
    if (n < 0 || n > DIST_N_MAX || x < 0 || x > n) return -1;
    if (dec_cmp(p, &zero) < 0 || dec_cmp(p, &one) > 0) return -1;
    xn_from_dec(px, p);
    return 0;
}

static dec_status binom_pmf_x(xnum_t *r, int64_t k, int64_t n, const xnum_t *p)
{
    xnum_t one, q, a, b, c, ln;
    xn_small(&one, 1);
    if (xn_is_zero(p)) { if (k == 0) *r = one; else xn_zero(r); return DEC_OK; }
    xn_sub(&q, &one, p);
    if (xn_is_zero(&q)) { if (k == n) *r = one; else xn_zero(r); return DEC_OK; }
    CHK(ln_fact(&a, n)); CHK(ln_fact(&b, k)); CHK(ln_fact(&c, n - k));
    xn_sub(&ln, &a, &b); xn_sub(&ln, &ln, &c);
    if (k > 0) { CHK(xf_ln(&a, p)); xn_mul_u32(&a, &a, (uint32_t)k); xn_add(&ln, &ln, &a); }
    if (n - k > 0) { CHK(xf_ln(&a, &q)); xn_mul_u32(&a, &a, (uint32_t)(n - k)); xn_add(&ln, &ln, &a); }
    return xf_exp(r, &ln);
}

dec_status dist_binom_pd(dec_t *r, int64_t x, int64_t n, const dec_t *p)
{
    xnum_t px, v;
    if (check_binom(x, n, p, &px)) { dec_zero(r); return DEC_DOMAIN; }
    CHK(binom_pmf_x(&v, x, n, &px));
    return finish(r, &v);
}

dec_status dist_binom_cd(dec_t *r, int64_t x, int64_t n, const dec_t *p)
{
    xnum_t px, one, q, t, s, ratio, mode, tmp;
    int64_t j, m;
    if (check_binom(x, n, p, &px)) { dec_zero(r); return DEC_DOMAIN; }
    xn_small(&one, 1);
    if (x == n) return dec_from_i64(r, 1);
    if (xn_is_zero(&px) || xn_cmp(&px, &one) == 0) {
        CHK(binom_pmf_x(&s, x, n, &px));
        if (xn_is_zero(&px)) return dec_from_i64(r, 1);          /* all mass at 0 <= x */
        return dec_from_i64(r, 0);                               /* all mass at n > x */
    }
    xn_sub(&q, &one, &px);
    xn_mul_u32(&mode, &px, (uint32_t)(n + 1));
    xn_to_i64(&mode, 0, &m);
    if (x <= m) {
        CHK(binom_pmf_x(&t, x, n, &px));
        s = t;
        for (j = x; j > 0; j--) {          /* pmf(j-1) = pmf(j) * j q / ((n-j+1) p) */
            xn_mul_u32(&t, &t, (uint32_t)j);
            xn_mul(&t, &t, &q);
            xn_mul_u32(&tmp, &px, (uint32_t)(n - j + 1));
            xn_div(&t, &t, &tmp);
            xn_add(&s, &s, &t);
            if (negligible(&t, &s)) break;
        }
        return finish(r, &s);
    }
    CHK(binom_pmf_x(&t, x + 1, n, &px));    /* upper tail: sum pmf(x+1..n) */
    s = t;
    for (j = x + 1; j < n; j++) {          /* pmf(j+1) = pmf(j) (n-j) p / ((j+1) q) */
        xn_mul_u32(&t, &t, (uint32_t)(n - j));
        xn_mul(&t, &t, &px);
        xn_mul_u32(&tmp, &q, (uint32_t)(j + 1));
        xn_div(&t, &t, &tmp);
        xn_add(&s, &s, &t);
        if (negligible(&t, &s)) break;
    }
    ratio = s;
    xn_sub(&s, &one, &ratio);
    return finish(r, &s);
}

/* ---- Poisson ---- */
static dec_status poisson_pmf_x(xnum_t *r, int64_t k, const xnum_t *lam)
{
    xnum_t ln, a, b;
    if (xn_is_zero(lam)) { if (k == 0) xn_small(r, 1); else xn_zero(r); return DEC_OK; }
    CHK(xf_ln(&ln, lam));
    xn_mul_u32(&ln, &ln, (uint32_t)k);
    CHK(ln_fact(&a, k));
    xn_sub(&ln, &ln, &a);
    xn_sub(&b, &ln, lam);
    return xf_exp(r, &b);
}

static int check_poisson(int64_t x, const dec_t *lam, xnum_t *lx)
{
    dec_t zero, lim;
    dec_zero(&zero); dec_from_i64(&lim, DIST_LAMBDA_MAX);
    if (x < 0 || x > 1000000000LL || dec_cmp(lam, &zero) < 0 || dec_cmp(lam, &lim) > 0) return -1;
    xn_from_dec(lx, lam);
    return 0;
}

dec_status dist_poisson_pd(dec_t *r, int64_t x, const dec_t *lam)
{
    xnum_t lx, v;
    if (check_poisson(x, lam, &lx)) { dec_zero(r); return DEC_DOMAIN; }
    CHK(poisson_pmf_x(&v, x, &lx));
    return finish(r, &v);
}

dec_status dist_poisson_cd(dec_t *r, int64_t x, const dec_t *lam)
{
    xnum_t lx, one, t, s, mode;
    int64_t j, m;
    if (check_poisson(x, lam, &lx)) { dec_zero(r); return DEC_DOMAIN; }
    xn_small(&one, 1);
    if (xn_is_zero(&lx)) return dec_from_i64(r, 1);
    xn_to_i64(&lx, 0, &m);
    mode = lx; (void)mode;
    if (x <= m) {
        CHK(poisson_pmf_x(&t, x, &lx));
        s = t;
        for (j = x; j > 0; j--) {          /* pmf(j-1) = pmf(j) * j / lambda */
            xn_mul_u32(&t, &t, (uint32_t)j);
            xn_div(&t, &t, &lx);
            xn_add(&s, &s, &t);
            if (negligible(&t, &s)) break;
        }
        return finish(r, &s);
    }
    CHK(poisson_pmf_x(&t, x + 1, &lx));     /* upper tail */
    s = t;
    for (j = x + 1; j < x + 1000000; j++) {
        xn_mul(&t, &t, &lx);
        xn_div_u32(&t, &t, (uint32_t)(j + 1));
        xn_add(&s, &s, &t);
        if (negligible(&t, &s)) break;
    }
    xn_sub(&t, &one, &s);
    return finish(r, &t);
}

/* ---- inverse normal ---- */
static dec_status norm_left_x(xnum_t *res, const xnum_t *x)
{
    xnum_t at, tail, one;
    xn_small(&one, 1);
    if (xn_is_zero(x)) { xn_small(res, 1); xn_div_u32(res, res, 2); return DEC_OK; }
    at = *x; at.neg = 0;
    CHK(tail_of(&tail, &at));
    if (x->neg) *res = tail; else xn_sub(res, &one, &tail);
    return DEC_OK;
}

dec_status dist_inv_norm(dec_t *r, const dec_t *area)
{
    xnum_t p, one, half, x, t, a, b, c, d, num, den, f, pdf, step, ln, sq2pi, tol, q;
    int flip = 0, it;
    dec_t zero, d1;
    dec_zero(&zero); dec_from_i64(&d1, 1);
    if (dec_cmp(area, &zero) <= 0 || dec_cmp(area, &d1) >= 0) { dec_zero(r); return DEC_DOMAIN; }
    xn_from_dec(&p, area);
    xn_small(&one, 1);
    xn_small(&half, 1); xn_div_u32(&half, &half, 2);
    if (xn_cmp(&p, &half) > 0) { xn_sub(&p, &one, &p); flip = 1; }
    if (xn_is_zero(&p)) { dec_zero(r); return DEC_DOMAIN; }
    /* Abramowitz & Stegun 26.2.23 starting value for the upper-tail quantile of p <= 1/2 */
    CHK(xf_ln(&ln, &p));
    xn_mul_u32(&ln, &ln, 2); xn_neg(&ln, &ln);
    xn_sqrt(&t, &ln);
    xn_small(&a, 2515517); xn_div_u32(&a, &a, 1000000);
    xn_small(&b, 802853);  xn_div_u32(&b, &b, 1000000);
    xn_small(&c, 10328);   xn_div_u32(&c, &c, 1000000);
    xn_mul(&num, &c, &t); xn_add(&num, &num, &b); xn_mul(&num, &num, &t); xn_add(&num, &num, &a);
    xn_small(&a, 1432788); xn_div_u32(&a, &a, 1000000);
    xn_small(&b, 189269);  xn_div_u32(&b, &b, 1000000);
    xn_small(&c, 1308);    xn_div_u32(&c, &c, 1000000);
    xn_mul(&den, &c, &t); xn_add(&den, &den, &b); xn_mul(&den, &den, &t); xn_add(&den, &den, &a);
    xn_mul(&den, &den, &t); xn_add(&den, &den, &one);
    xn_div(&q, &num, &den);
    xn_sub(&x, &t, &q);                    /* upper-tail quantile: R(x) = p */
    xn_neg(&x, &x);                        /* left-tail: P(x) = p, x <= 0 */
    xn_mul_u32(&q, &XC_PI, 2); xn_sqrt(&sq2pi, &q);
    xn_pow10(&tol, -26);
    for (it = 0; it < 30; it++) {
        CHK(norm_left_x(&f, &x));
        xn_sub(&f, &f, &p);
        xn_mul(&q, &x, &x); xn_div_u32(&q, &q, 2); xn_neg(&q, &q);
        CHK(xf_exp(&pdf, &q));
        xn_div(&pdf, &pdf, &sq2pi);
        if (xn_is_zero(&pdf)) break;
        xn_div(&step, &f, &pdf);
        xn_sub(&x, &x, &step);
        xn_abs(&step, &step);
        xn_small(&d, 1);
        if (xn_cmp(&step, &tol) < 0) break;
    }
    if (flip) xn_neg(&x, &x);
    if (xn_is_zero(&x)) { dec_zero(r); return DEC_OK; }
    return xn_to_dec(r, &x);
}
