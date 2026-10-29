#include <string.h>
#include "solvers/eqn.h"
#include "values/matrix.h"
#include "calc/nummethods.h"

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

calc_err_t eqn_simul(int n, const num_t *a, num_t *x)
{
    num_t m[9], rhs[3], inv[9];
    mv_t A, I, B, X;
    int i, j;
    if (n < 2 || n > 3) return CE_ARG;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) m[i * n + j] = a[i * (n + 1) + j];
        rhs[i] = a[i * (n + 1) + n];
    }
    A.r = A.c = (uint8_t)n; A.e = m;
    I.e = inv;
    B.r = (uint8_t)n; B.c = 1; B.e = rhs;
    X.e = x;
    CHK(mx_inv(&I, &A));
    CHK(mx_mul(&X, &I, &B));
    return CE_OK;
}
