#include <string.h>
#include "calc/nummethods.h"
#include "calc/format.h"

typedef struct nm {
    dec_status st;
    uint32_t   evals;
} nm_t;

#define MAX_EVALS 60000u

static dec_t dc(int64_t n) { dec_t r; dec_from_i64(&r, n); return r; }
static dec_t ds(const char *s) { dec_t r; dec_from_str(&r, s); return r; }

static dec_t add(nm_t *n, dec_t a, dec_t b) { dec_t r; dec_status s = dec_add(&r, &a, &b); if (!n->st) n->st = s; return r; }
static dec_t sub(nm_t *n, dec_t a, dec_t b) { dec_t r; dec_status s = dec_sub(&r, &a, &b); if (!n->st) n->st = s; return r; }
static dec_t mul(nm_t *n, dec_t a, dec_t b) { dec_t r; dec_status s = dec_mul(&r, &a, &b); if (!n->st) n->st = s; return r; }
static dec_t dv(nm_t *n, dec_t a, dec_t b) { dec_t r; dec_status s = dec_div(&r, &a, &b); if (!n->st) n->st = s; return r; }
static dec_t dabs(dec_t a) { dec_t r; dec_abs(&r, &a); return r; }
static int lt(dec_t a, dec_t b) { return dec_cmp(&a, &b) < 0; }
static int gt(dec_t a, dec_t b) { return dec_cmp(&a, &b) > 0; }
static int le(dec_t a, dec_t b) { return dec_cmp(&a, &b) <= 0; }
static int sgn(dec_t a) { return dec_is_zero(&a) ? 0 : (a.neg ? -1 : 1); }

static calc_err_t status_err(dec_status s)
{
    if (s == DEC_OK || s == DEC_UNDERFLOW) return CE_OK;
    return CE_MATH;
}

static calc_err_t call(nm_t *n, nm_fn f, void *u, dec_t x, dec_t *y)
{
    calc_err_t e;
    if (++n->evals > MAX_EVALS) return CE_TIME;
    e = f(u, &x, y);
    return e;
}

/* ------------------------------------------------------------------ */
/* derivative: Ridders' extrapolation                                  */
/* ------------------------------------------------------------------ */

#define NTAB 10

calc_err_t nm_derivative(nm_fn f, void *user, const dec_t *x0, dec_t *out)
{
    nm_t n;
    dec_t a[NTAB][NTAB], x = *x0, h, hh, con, con2, fac, err, ans, safe, two;
    dec_t ax;
    int i, j;
    calc_err_t e;

    memset(&n, 0, sizeof n);
    con = ds("1.4");
    con2 = ds("1.96");
    safe = dc(2);
    two = dc(2);
    ax = dabs(x);
    if (lt(ax, dc(1))) ax = dc(1);
    h = mul(&n, ax, ds("0.05"));
    /* make the step exactly representable */
    hh = sub(&n, add(&n, x, h), x);
    if (dec_is_zero(&hh)) hh = h;
    h = hh;

    {
        dec_t fp, fm;
        e = call(&n, f, user, add(&n, x, hh), &fp); if (e) return e;
        e = call(&n, f, user, sub(&n, x, hh), &fm); if (e) return e;
        a[0][0] = dv(&n, sub(&n, fp, fm), mul(&n, two, hh));
    }
    err = ds("1E90");
    ans = a[0][0];
    for (i = 1; i < NTAB; i++) {
        dec_t fp, fm, errt, d1, d2;
        hh = dv(&n, hh, con);
        e = call(&n, f, user, add(&n, x, hh), &fp); if (e) return e;
        e = call(&n, f, user, sub(&n, x, hh), &fm); if (e) return e;
        a[0][i] = dv(&n, sub(&n, fp, fm), mul(&n, two, hh));
        fac = con2;
        for (j = 1; j <= i; j++) {
            a[j][i] = dv(&n, sub(&n, mul(&n, a[j - 1][i], fac), a[j - 1][i - 1]), sub(&n, fac, dc(1)));
            fac = mul(&n, con2, fac);
            d1 = dabs(sub(&n, a[j][i], a[j - 1][i]));
            d2 = dabs(sub(&n, a[j][i], a[j - 1][i - 1]));
            errt = gt(d1, d2) ? d1 : d2;
            if (le(errt, err)) { err = errt; ans = a[j][i]; }
        }
        if (n.st) return status_err(n.st);
        if (!lt(dabs(sub(&n, a[i][i], a[i - 1][i - 1])), mul(&n, safe, err))) break;
    }
    if (status_err(n.st)) return CE_MATH;
    {
        dec_t ref = dabs(a[0][0]);
        if (lt(dabs(ans), err) || lt(dabs(ans), mul(&n, ref, ds("1E-12")))) dec_zero(&ans);   /* indistinguishable from zero */
    }
    *out = ans;
    return CE_OK;
}

/* ------------------------------------------------------------------ */
/* integral: adaptive Gauss-Kronrod (7, 15)                            */
/* ------------------------------------------------------------------ */

#define GK_N 8
static const char *const XGK[GK_N] = {
    "0.991455371120812639206854697526329", "0.949107912342758524526189684047851",
    "0.864864423359769072789712788640926", "0.741531185599394439863864773280788",
    "0.586087235467691130294144838258730", "0.405845151377397166906606412076961",
    "0.207784955007898467600689403773245", "0"
};
static const char *const WGK[GK_N] = {
    "0.022935322010529224963732008058970", "0.063092092629978553290700663189204",
    "0.104790010322250183839876322541518", "0.140653259715525918745189590510238",
    "0.169004726639267902826583426598550", "0.190350578064785409913256402421014",
    "0.204432940075298892414161999234649", "0.209482141084727828012999174891714"
};
static const char *const WG[4] = {
    "0.129484966168869693270611432679082", "0.279705391489276667901467771423780",
    "0.381830050505118944950369775488975", "0.417959183673469387755102040816327"
};

#define MAX_IV 80

typedef struct iv {
    dec_t a, b, r, e;
} iv_t;

static calc_err_t gk15(nm_t *n, nm_fn f, void *u, dec_t a, dec_t b, dec_t *res, dec_t *err, dec_t *resabs)
{
    dec_t c, hl, resk, resg, fc, asc, half = ds("0.5");
    int i;
    calc_err_t e;

    c = mul(n, add(n, a, b), half);
    hl = mul(n, sub(n, b, a), half);
    e = call(n, f, u, c, &fc);
    if (e) return e;
    resk = mul(n, ds(WGK[7]), fc);
    resg = mul(n, ds(WG[3]), fc);
    *resabs = dabs(resk);
    {
        dec_t fv1[7], fv2[7];
        for (i = 0; i < 7; i++) {
            dec_t dx = mul(n, hl, ds(XGK[i]));
            dec_t f1, f2, sum;
            e = call(n, f, u, sub(n, c, dx), &f1); if (e) return e;
            e = call(n, f, u, add(n, c, dx), &f2); if (e) return e;
            fv1[i] = f1; fv2[i] = f2;
            sum = add(n, f1, f2);
            resk = add(n, resk, mul(n, ds(WGK[i]), sum));
            *resabs = add(n, *resabs, mul(n, ds(WGK[i]), add(n, dabs(f1), dabs(f2))));
            if (i & 1) resg = add(n, resg, mul(n, ds(WG[i >> 1]), sum));
        }
        /* mean value for the scaled error estimate */
        {
            dec_t reskh = mul(n, resk, half);
            asc = mul(n, ds(WGK[7]), dabs(sub(n, fc, reskh)));
            for (i = 0; i < 7; i++)
                asc = add(n, asc, mul(n, ds(WGK[i]), add(n, dabs(sub(n, fv1[i], reskh)), dabs(sub(n, fv2[i], reskh)))));
        }
    }
    *res = mul(n, resk, hl);
    *resabs = mul(n, *resabs, dabs(hl));
    asc = mul(n, asc, dabs(hl));
    *err = dabs(mul(n, sub(n, resk, resg), hl));
    if (!dec_is_zero(&asc) && !dec_is_zero(err)) {
        dec_t r = dv(n, mul(n, dc(200), *err), asc);
        if (lt(r, dc(1))) {
            dec_t sq;
            dec_status st = dec_sqrt(&sq, &r);
            if (!n->st) n->st = st;
            *err = mul(n, asc, mul(n, r, sq));
        } else {
            *err = asc;
        }
    }
    {
        dec_t floor_ = mul(n, *resabs, ds("5E-16"));
        if (lt(*err, floor_)) *err = floor_;
    }
    if (n->st) return status_err(n->st);
    return CE_OK;
}

calc_err_t nm_integrate(nm_fn f, void *user, const dec_t *a, const dec_t *b, dec_t *out)
{
    static iv_t iv[MAX_IV];
    nm_t n;
    int cnt = 1, i, iter;
    dec_t total, etotal, resabs_total, tol;
    calc_err_t e;
    dec_t ra;

    memset(&n, 0, sizeof n);
    if (dec_cmp(a, b) == 0) { dec_zero(out); return CE_OK; }
    iv[0].a = *a; iv[0].b = *b;
    e = gk15(&n, f, user, *a, *b, &iv[0].r, &iv[0].e, &ra);
    if (e) return e;
    resabs_total = ra;
    for (iter = 0; iter < MAX_IV; iter++) {
        int worst = 0;
        total = dc(0); etotal = dc(0);
        for (i = 0; i < cnt; i++) {
            total = add(&n, total, iv[i].r);
            etotal = add(&n, etotal, iv[i].e);
            if (gt(iv[i].e, iv[worst].e)) worst = i;
        }
        tol = mul(&n, dabs(total), ds("1E-12"));
        {
            dec_t t2 = mul(&n, resabs_total, ds("1E-15"));
            if (gt(t2, tol)) tol = t2;
        }
        if (n.st) return status_err(n.st);
        if (le(etotal, tol) || cnt >= MAX_IV) break;
        /* bisect the interval with the largest error */
        {
            dec_t m = mul(&n, add(&n, iv[worst].a, iv[worst].b), ds("0.5"));
            iv_t left, right;
            dec_t ra1, ra2;
            left.a = iv[worst].a; left.b = m;
            right.a = m; right.b = iv[worst].b;
            e = gk15(&n, f, user, left.a, left.b, &left.r, &left.e, &ra1); if (e) return e;
            e = gk15(&n, f, user, right.a, right.b, &right.r, &right.e, &ra2); if (e) return e;
            iv[worst] = left;
            iv[cnt++] = right;
        }
        /* recompute |f| integral estimate from the intervals */
        resabs_total = dc(0);
        for (i = 0; i < cnt; i++) resabs_total = add(&n, resabs_total, dabs(iv[i].r));
    }
    if (n.st) return status_err(n.st);
    *out = total;
    return CE_OK;
}

/* ------------------------------------------------------------------ */
/* root finding                                                         */
/* ------------------------------------------------------------------ */

/* Brent's method on a bracket [a, b] with f(a) f(b) < 0. */
static calc_err_t brent(nm_t *n, nm_fn f, void *u, dec_t a, dec_t b, dec_t fa, dec_t fb, dec_t *root, dec_t *froot)
{
    dec_t c = b, fc = fb, d = dc(0), e_ = dc(0), eps = ds("2E-16");
    int iter;
    calc_err_t er;

    for (iter = 0; iter < 200; iter++) {
        dec_t tol1, xm, p, q, r, s_;
        if ((sgn(fb) > 0) == (sgn(fc) > 0) && sgn(fb) != 0) {
            c = a; fc = fa; d = sub(n, b, a); e_ = d;
        }
        if (lt(dabs(fc), dabs(fb))) {
            a = b; b = c; c = a;
            fa = fb; fb = fc; fc = fa;
        }
        tol1 = mul(n, mul(n, dc(2), eps), dabs(b));
        xm = mul(n, sub(n, c, b), ds("0.5"));
        if (dec_is_zero(&fb) || le(dabs(xm), tol1)) {
            *root = b; *froot = fb;
            return n->st ? CE_MATH : CE_OK;
        }
        if (!lt(dabs(e_), tol1) && gt(dabs(fa), dabs(fb))) {
            s_ = dv(n, fb, fa);
            if (dec_cmp(&a, &c) == 0) {
                p = mul(n, mul(n, dc(2), xm), s_);
                q = sub(n, dc(1), s_);
            } else {
                dec_t qq = dv(n, fa, fc), rr = dv(n, fb, fc);
                p = mul(n, s_, sub(n, mul(n, mul(n, dc(2), xm), mul(n, qq, sub(n, qq, rr))), mul(n, sub(n, b, a), sub(n, rr, dc(1)))));
                q = mul(n, mul(n, sub(n, qq, dc(1)), sub(n, rr, dc(1))), sub(n, s_, dc(1)));
            }
            if (sgn(p) > 0) q = mul(n, q, dc(-1)); else p = dabs(p);
            {
                dec_t m1 = mul(n, mul(n, dc(3), xm), q);
                dec_t m1b = sub(n, m1, dabs(mul(n, tol1, q)));
                dec_t m2 = dabs(mul(n, e_, q));
                dec_t lim = lt(m1b, m2) ? m1b : m2;
                if (lt(mul(n, dc(2), p), lim)) { e_ = d; d = dv(n, p, q); }
                else { d = xm; e_ = d; }
            }
        } else {
            d = xm; e_ = d;
        }
        a = b; fa = fb;
        r = dabs(d);
        if (gt(r, tol1)) b = add(n, b, d);
        else b = add(n, b, sgn(xm) >= 0 ? tol1 : mul(n, tol1, dc(-1)));
        er = call(n, f, u, b, &fb);
        if (er) return er;
        if (n->st) return CE_MATH;
    }
    *root = b; *froot = fb;
    return CE_OK;
}

static calc_err_t nm_solve_raw(nm_fn f, void *user, const dec_t *guess, dec_t *root, dec_t *resid);

/* Prefer a short decimal root when it is at least as good a root as the computed one. */
calc_err_t nm_solve(nm_fn f, void *user, const dec_t *guess, dec_t *root, dec_t *resid)
{
    calc_err_t e = nm_solve_raw(f, user, guess, root, resid);
    int k;
    if (e) return e;
    for (k = 6; k < 16; k++) {
        dec_t c, fc;
        if (dec_round_sig(&c, root, k) != DEC_OK) break;
        if (f(user, &c, &fc) != CE_OK) continue;
        if (dec_cmp(&c, root) == 0) break;
        {
            dec_t a1 = fc, a2 = *resid;
            dec_abs(&a1, &a1); dec_abs(&a2, &a2);
            if (dec_cmp(&a1, &a2) <= 0) { *root = c; *resid = fc; break; }
        }
    }
    return CE_OK;
}

static calc_err_t nm_solve_raw(nm_fn f, void *user, const dec_t *guess, dec_t *root, dec_t *resid)
{
    nm_t n;
    dec_t x0 = *guess, f0, h, scale;
    int side, k;
    dec_t prev[2], fprev[2];
    int pvalid[2];
    calc_err_t e;
    int f0_ok;

    memset(&n, 0, sizeof n);
    e = call(&n, f, user, x0, &f0);
    f0_ok = (e == CE_OK);
    if (e == CE_TIME) return e;
    if (f0_ok && dec_is_zero(&f0)) { *root = x0; dec_zero(resid); return CE_OK; }

    scale = dabs(x0);
    if (lt(scale, dc(1))) scale = dc(1);
    h = mul(&n, scale, ds("0.01"));

    for (side = 0; side < 2; side++) { prev[side] = x0; fprev[side] = f0; pvalid[side] = f0_ok; }

    {
        dec_t best_r = dc(0), best_f = dc(0), best_d = dc(0);
        int have = 0;
        for (k = 0; k < 220; k++) {
            for (side = 0; side < 2; side++) {
                dec_t x, fx;
                x = side == 0 ? add(&n, x0, h) : sub(&n, x0, h);
                if (n.st) { n.st = DEC_OK; continue; }
                e = call(&n, f, user, x, &fx);
                if (e == CE_TIME) return e;
                if (e) { pvalid[side] = 0; continue; }
                if (dec_is_zero(&fx)) {
                    dec_t d = dabs(sub(&n, x, x0));
                    if (!have || lt(d, best_d)) { best_r = x; best_f = fx; best_d = d; have = 1; }
                } else if (pvalid[side] && sgn(fx) != sgn(fprev[side])) {
                    dec_t r, fr;
                    e = brent(&n, f, user, prev[side], x, fprev[side], fx, &r, &fr);
                    if (e == CE_TIME) return e;
                    if (!e) {
                        /* reject poles: the residual must be small compared with the bracket values */
                        dec_t m = dabs(fprev[side]);
                        if (lt(m, dabs(fx))) m = dabs(fx);
                        if (lt(dabs(fr), mul(&n, m, ds("1E-6")))) {
                            dec_t d = dabs(sub(&n, r, x0));
                            if (!have || lt(d, best_d)) { best_r = r; best_f = fr; best_d = d; have = 1; }
                        }
                    }
                }
                prev[side] = x; fprev[side] = fx; pvalid[side] = 1;
            }
            if (have) break;      /* both sides of this step are done: nearest candidate wins */
            h = mul(&n, h, ds("1.3"));
            if (n.st) break;
            if (gt(h, ds("1E15"))) break;
        }
        if (have) { *root = best_r; *resid = best_f; return CE_OK; }
    }

    /* no sign change: Newton with multiplicity 1..4 (even-order roots) */
    if (f0_ok) {
        int mult;
        dec_t best_x = x0, best_f = f0;
        int have = 0;
        for (mult = 1; mult <= 4; mult++) {
            dec_t x = x0, fx = f0;
            int it;
            n.st = DEC_OK;
            for (it = 0; it < 100; it++) {
                dec_t df, step, hh2 = mul(&n, scale, ds("1E-7")), f2, xn, fxn;
                e = call(&n, f, user, add(&n, x, hh2), &f2);
                if (e) break;
                df = dv(&n, sub(&n, f2, fx), hh2);
                if (n.st || dec_is_zero(&df)) break;
                step = mul(&n, dc(mult), dv(&n, fx, df));
                xn = sub(&n, x, step);
                e = call(&n, f, user, xn, &fxn);
                if (e || n.st) break;
                x = xn; fx = fxn;
                if (dec_is_zero(&fx)) break;
            }
            if (!n.st && le(dabs(fx), mul(&n, add(&n, dc(1), dabs(f0)), ds("1E-13")))) {
                if (!have || lt(dabs(fx), dabs(best_f)) || dec_is_zero(&fx)) { best_x = x; best_f = fx; have = 1; }
            }
        }
        if (have) { *root = best_x; *resid = best_f; return CE_OK; }
    }
    return CE_NOSOL;
}
