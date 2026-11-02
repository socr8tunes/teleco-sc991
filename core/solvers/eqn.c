#include <string.h>
#include "solvers/eqn.h"
#include "values/matrix.h"
#include "calc/nummethods.h"
#include "numeric/xnum.h"

#define CHK(x) do { dec_status _s = (x); if (DEC_STATUS_IS_ERROR(_s)) return CE_MATH; } while (0)

calc_err_t eqn_quad(const num_t c[3], cx_t roots[2])
{
    num_t a = c[0], b = c[1], cc = c[2], d, t, u, two, den, nb, sq;
    if (num_is_zero(&a)) return CE_MATH;
    num_from_int(&two, 2);
    CHK(num_mul(&t, &b, &b));
    CHK(num_mul(&u, &a, &cc));
    num_from_int(&sq, 4);
    CHK(num_mul(&u, &u, &sq));
    CHK(num_sub(&d, &t, &u));                 /* discriminant */
    CHK(num_mul(&den, &two, &a));
    CHK(num_neg(&nb, &b));
    if (!d.v.neg || dec_is_zero(&d.v)) {
        CHK(num_sqrt(&sq, &d));
        CHK(num_add(&t, &nb, &sq));
        CHK(num_div(&roots[0].re, &t, &den));
        CHK(num_sub(&t, &nb, &sq));
        CHK(num_div(&roots[1].re, &t, &den));
        num_from_int(&roots[0].im, 0);
        num_from_int(&roots[1].im, 0);
    } else {
        num_t nd;
        CHK(num_neg(&nd, &d));
        CHK(num_sqrt(&sq, &nd));
        CHK(num_div(&t, &nb, &den));
        CHK(num_div(&u, &sq, &den));
        roots[0].re = t; roots[0].im = u;
        roots[1].re = t;
        CHK(num_neg(&roots[1].im, &u));
    }
    return CE_OK;
}

/* ---- cubic ----------------------------------------------------------------- */

typedef struct poly { dec_t c[4]; } poly_t;

static calc_err_t poly_cb(void *user, const dec_t *x, dec_t *y)
{
    const poly_t *p = (const poly_t *)user;
    dec_t acc = p->c[0];
    int i;
    for (i = 1; i < 4; i++) {
        dec_status s = dec_mul(&acc, &acc, x);
        if (!s) s = dec_add(&acc, &acc, &p->c[i]);
        if (DEC_STATUS_IS_ERROR(s)) return CE_MATH;
    }
    *y = acc;
    return CE_OK;
}

static int all_int(const num_t c[4], int64_t iv[4])
{
    int i;
    for (i = 0; i < 4; i++) {
        if (!num_is_exact(&c[i]) || !ex_is_integer(&c[i].ex) || c[i].ex.a > 1000000000000LL || c[i].ex.a < -1000000000000LL) return 0;
        iv[i] = c[i].ex.a;
    }
    return 1;
}

/* exact rational root search by the rational root theorem */
static int test_root(const int64_t iv[4], int64_t p, int64_t q, num_t *out)
{
    /* x = p/q: a p^3 + b p^2 q + c p q^2 + d q^3 == 0, evaluated with checked num arithmetic */
    num_t x, acc, t;
    int i;
    num_from_int(&x, p);
    num_from_int(&t, q);
    if (num_div(&x, &x, &t) != DEC_OK) return 0;
    num_from_int(&acc, iv[0]);
    for (i = 1; i < 4; i++) {
        num_t k;
        if (num_mul(&acc, &acc, &x) != DEC_OK) return 0;
        num_from_int(&k, iv[i]);
        if (num_add(&acc, &acc, &k) != DEC_OK) return 0;
    }
    if (!num_is_exact(&acc) || !num_is_zero(&acc)) return 0;
    *out = x;
    return 1;
}

static int divisors(int64_t n, int64_t *out, int cap)
{
    int cnt = 0;
    int64_t i;
    for (i = 1; i * i <= n; i++) {
        if (n % i) continue;
        if (cnt >= cap) return -1;
        out[cnt++] = i;
        if (i != n / i) {
            if (cnt >= cap) return -1;
            out[cnt++] = n / i;
        }
    }
    return cnt;
}

static int rational_root(const int64_t iv[4], num_t *root)
{
    int64_t a = iv[0] < 0 ? -iv[0] : iv[0], d = iv[3] < 0 ? -iv[3] : iv[3];
    int64_t dp[256], dq[256];
    int np, nq, i, j;
    if (d == 0) { num_from_int(root, 0); return 1; }
    if (d > 1000000000LL || a > 1000000000LL) return 0;
    np = divisors(d, dp, 256);
    nq = divisors(a, dq, 256);
    if (np < 0 || nq < 0) return 0;
    for (i = 0; i < nq; i++)
        for (j = 0; j < np; j++) {
            if (test_root(iv, dp[j], dq[i], root)) return 1;
            if (test_root(iv, -dp[j], dq[i], root)) return 1;
        }
    return 0;
}

calc_err_t eqn_cubic(const num_t c[4], cx_t roots[3])
{
    num_t r, b2, b1, b0, t;
    cx_t qr[2], all[3];
    num_t qc[3];
    int64_t iv[4];
    int found = 0, i, j;
    calc_err_t e;

    if (num_is_zero(&c[0])) return CE_MATH;
    if (all_int(c, iv) && rational_root(iv, &r)) found = 1;
    if (!found) {
        poly_t p;
        dec_t root, resid, guess;
        for (i = 0; i < 4; i++) p.c[i] = c[i].v;
        dec_zero(&guess);
        e = nm_solve(poly_cb, &p, &guess, &root, &resid);
        if (e) return e;
        num_from_dec(&r, &root);
    }
    /* deflate: a x^2 + (b + r a) x + (c + r (b + r a)) */
    b2 = c[0];
    CHK(num_mul(&t, &r, &b2)); CHK(num_add(&b1, &c[1], &t));
    CHK(num_mul(&t, &r, &b1)); CHK(num_add(&b0, &c[2], &t));
    qc[0] = b2; qc[1] = b1; qc[2] = b0;
    e = eqn_quad(qc, qr);
    if (e) return e;
    cx_from_real(&all[0], &r);
    all[1] = qr[0]; all[2] = qr[1];
    /* real roots first, ascending */
    for (i = 0; i < 3; i++)
        for (j = i + 1; j < 3; j++) {
            int ri = num_is_zero(&all[i].im), rj = num_is_zero(&all[j].im);
            int swap = 0;
            if (!ri && rj) swap = 1;
            else if (ri && rj && num_cmp(&all[i].re, &all[j].re) > 0) swap = 1;
            if (swap) { cx_t tmp = all[i]; all[i] = all[j]; all[j] = tmp; }
        }
    memcpy(roots, all, sizeof all);
    return CE_OK;
}

/*
 * n x n linear system (n = 2..4), a = n rows of (n coefficients + constant).
 *
 * Each row is first scaled by a power of ten so that decimal inputs become integers (this does not change the
 * solution). Fraction-free Gauss-Jordan (Bareiss) elimination with partial pivoting then runs in 45+ digit
 * extended precision: for the integer entries produced above every intermediate value is an exact minor, so
 * rank decisions are exact (the zero test is only a relative safety net for entries beyond ~12 digits).
 * Unique solution -> CE_OK (exact rational when numerator and denominator fit in int64, else a 16-digit
 * decimal rounded once from the extended precision value); inconsistent -> CE_INCONS; underdetermined -> CE_INFIN.
 */
calc_err_t eqn_simul(int n, const num_t *a, num_t *x)
{
    xnum_t m[4][5], prev, tol, scale, eps, t, u, piv, zero;
    int pcol[4] = {0, 0, 0, 0};
    int r = 0, col, i, j, w = n + 1, exact_mode;
    if (n < 2 || n > 4) return CE_ARG;
    xn_zero(&zero);
    for (i = 0; i < n; i++) {
        int k;
        for (j = 0; j < w; j++) xn_from_dec(&m[i][j], &a[i * w + j].v);
        for (k = 0; k < 40; k++) {                       /* scale the row to integers */
            int all = 1;
            for (j = 0; j < w && all; j++) all = xn_is_int(&m[i][j]);
            if (all) break;
            for (j = 0; j < w; j++) xn_mul_u32(&m[i][j], &m[i][j], 10);
        }
    }
    xn_zero(&scale);
    for (i = 0; i < n; i++)
        for (j = 0; j < w; j++) { xn_abs(&t, &m[i][j]); if (xn_cmp(&t, &scale) > 0) scale = t; }
    if (xn_is_zero(&scale)) return CE_INFIN;                   /* 0 = 0 */
    /* Minors of order 4 of d-digit integers have at most 4d+2 digits; with 45+ digits of precision the
     * elimination is exact when d <= 10, and then zero means exactly zero. Beyond that a relative test is used. */
    exact_mode = xn_dec_exponent(&scale) + 1 <= 10;
    xn_from_i64(&prev, 1);
    xn_pow10(&eps, -30);
    xn_zero(&tol);
    for (col = 0; col < n && r < n; col++) {
        int p = -1;
        xnum_t best = zero;
        if (!exact_mode) {                                     /* relative zero threshold from the current magnitudes */
            xnum_t mx = zero;
            for (i = r; i < n; i++) for (j = 0; j < w; j++) { xn_abs(&t, &m[i][j]); if (xn_cmp(&t, &mx) > 0) mx = t; }
            xn_mul(&tol, &mx, &eps);
        }
        for (i = r; i < n; i++) {
            xn_abs(&t, &m[i][col]);
            if (xn_cmp(&t, &tol) > 0 && xn_cmp(&t, &best) > 0) { best = t; p = i; }
        }
        if (p < 0) { for (i = r; i < n; i++) xn_zero(&m[i][col]); continue; }
        if (p != r) for (j = 0; j < w; j++) { t = m[p][j]; m[p][j] = m[r][j]; m[r][j] = t; }
        piv = m[r][col];
        for (i = 0; i < n; i++) {
            if (i == r) continue;
            for (j = 0; j < w; j++) {
                if (j == col) continue;
                xn_mul(&t, &piv, &m[i][j]);
                xn_mul(&u, &m[i][col], &m[r][j]);
                xn_sub(&t, &t, &u);
                xn_div(&m[i][j], &t, &prev);
            }
            xn_zero(&m[i][col]);
        }
        pcol[r] = col;
        prev = piv;
        r++;
    }
    if (!exact_mode) {
        xnum_t mx = zero;
        for (i = r; i < n; i++) for (j = 0; j < w; j++) { xn_abs(&t, &m[i][j]); if (xn_cmp(&t, &mx) > 0) mx = t; }
        xn_mul(&tol, &mx, &eps);
    }
    for (i = r; i < n; i++) {                                  /* rows without coefficients must read 0 = 0 */
        xn_abs(&t, &m[i][n]);
        if (xn_cmp(&t, &tol) > 0) return CE_INCONS;
    }
    if (r < n) return CE_INFIN;
    for (i = 0; i < n; i++) {
        int64_t nn, dd;
        dec_t d;
        num_t pn, pd;
        xnum_t q;
        const xnum_t *den = &m[i][pcol[i]];
        if (xn_is_int(&m[i][n]) && xn_is_int(den) && !xn_to_i64(&m[i][n], XN_TRUNC, &nn) && !xn_to_i64(den, XN_TRUNC, &dd)) {
            num_from_int(&pn, nn);
            num_from_int(&pd, dd);
            if (!DEC_STATUS_IS_ERROR(num_div(&x[i], &pn, &pd))) continue;
        }
        xn_div(&q, &m[i][n], den);
        if (DEC_STATUS_IS_ERROR(xn_to_dec(&d, &q))) return CE_MATH;
        num_from_dec(&x[i], &d);
    }
    return CE_OK;
}
