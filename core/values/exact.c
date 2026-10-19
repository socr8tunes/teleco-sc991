#include "values/exact.h"

#include <string.h>
#include "numeric/wide.h"
#include "numeric/xnum.h"
#include "numeric/xconst.h"

/* ------------------------------------------------------------------------ */
/* checked 64-bit arithmetic                                                */
/* ------------------------------------------------------------------------ */

static uint64_t umag(int64_t v)
{
    return v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
}

static int mul64(int64_t a, int64_t b, int64_t *r)
{
    u128 p = u128_mul_u64(umag(a), umag(b));

    if (p.hi != 0 || p.lo > (uint64_t)INT64_MAX) {
        return -1;
    }
    *r = ((a < 0) != (b < 0)) ? -(int64_t)p.lo : (int64_t)p.lo;
    return 0;
}

static int add64(int64_t a, int64_t b, int64_t *r)
{
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < -INT64_MAX - b)) {
        return -1;
    }
    *r = a + b;
    return 0;
}

static uint64_t gcd64(uint64_t a, uint64_t b)
{
    while (b != 0) {
        uint64_t t = a % b;
        a = b;
        b = t;
    }
    return a;
}

/* ------------------------------------------------------------------------ */
/* construction and normalisation                                           */
/* ------------------------------------------------------------------------ */

void ex_none(ex_t *r)
{
    r->kind = EX_NONE;
    r->a = 0;
    r->b = 0;
    r->d = 1;
    r->s = 1;
}

static void normalize(ex_t *x)
{
    uint64_t g;

    if (x->d < 0) {
        x->a = -x->a;
        x->b = -x->b;
        x->d = -x->d;
    }
    if (x->kind == EX_PI) {
        x->b = 0;
        x->s = 1;
    }
    if (x->b == 0) {
        x->s = 1;
    }
    g = gcd64(gcd64(umag(x->a), umag(x->b)), (uint64_t)x->d);
    if (g > 1) {
        x->a /= (int64_t)g;
        x->b /= (int64_t)g;
        x->d /= (int64_t)g;
    }
    if (x->a == 0 && x->b == 0) {
        x->d = 1;
        x->s = 1;
    }
}

void ex_int(ex_t *r, int64_t n)
{
    r->kind = EX_QS;
    r->a = n;
    r->b = 0;
    r->d = 1;
    r->s = 1;
}

int ex_rat(ex_t *r, int64_t num, int64_t den)
{
    if (den == 0 || num == INT64_MIN || den == INT64_MIN) {
        ex_none(r);
        return -1;
    }
    r->kind = EX_QS;
    r->a = num;
    r->b = 0;
    r->d = den;
    r->s = 1;
    normalize(r);
    return 0;
}

void ex_pi(ex_t *r, int64_t num, int64_t den)
{
    r->kind = EX_PI;
    r->a = num;
    r->b = 0;
    r->d = den;
    r->s = 1;
    normalize(r);
}

int ex_valid(const ex_t *x)
{
    return x->kind != EX_NONE;
}

int ex_is_rational(const ex_t *x)
{
    return x->kind == EX_QS && x->b == 0;
}

int ex_is_integer(const ex_t *x)
{
    return ex_is_rational(x) && x->d == 1;
}

int ex_equal(const ex_t *x, const ex_t *y)
{
    return x->kind == y->kind && x->a == y->a && x->b == y->b && x->d == y->d && x->s == y->s;
}

int ex_sign(const ex_t *x)
{
    u128 a2, b2s;

    if (x->kind == EX_NONE) {
        return 0;
    }
    if (x->kind == EX_PI || x->b == 0) {
        return x->a > 0 ? 1 : (x->a < 0 ? -1 : 0);
    }
    if (x->a == 0) {
        return x->b > 0 ? 1 : -1;
    }
    if ((x->a > 0) == (x->b > 0)) {
        return x->a > 0 ? 1 : -1;
    }
    a2 = u128_mul_u64(umag(x->a), umag(x->a));
    b2s = u128_mulw(u128_mul_u64(umag(x->b), umag(x->b)), (uint64_t)x->s);
    if (u128_cmp(a2, b2s) == 0) {
        return 0; /* cannot happen for squarefree s > 1 */
    }
    /* signs differ: the larger magnitude wins */
    if (u128_cmp(a2, b2s) > 0) {
        return x->a > 0 ? 1 : -1;
    }
    return x->b > 0 ? 1 : -1;
}

/* ------------------------------------------------------------------------ */
/* square-free split and integer roots                                      */
/* ------------------------------------------------------------------------ */

int ex_sqrt_split(uint64_t n, uint64_t *k, uint64_t *s)
{
    uint64_t rem = n;
    uint64_t kk = 1;
    uint64_t ss = 1;
    uint64_t p;

    if (n > EX_SQRT_LIMIT || n == 0) {
        return -1;
    }
    for (p = 2; p * p <= rem; p += (p == 2) ? 1u : 2u) {
        int cnt = 0;
        while (rem % p == 0) {
            rem /= p;
            cnt++;
        }
        while (cnt >= 2) {
            kk *= p;
            cnt -= 2;
        }
        if (cnt == 1) {
            ss *= p;
        }
    }
    ss *= rem;
    *k = kk;
    *s = ss;
    return 0;
}

/* floor(v^(1/n)) by binary search, exact flag */
static int iroot_exact(uint64_t v, int64_t n, uint64_t *root)
{
    uint64_t lo = 0, hi = 3037000500ULL; /* > sqrt(2^63) */
    int i;

    if (v <= 1) {
        *root = v;
        return 1;
    }
    if (n >= 63) {
        return 0;
    }
    if (n >= 3) {
        hi = 2097152ULL; /* 2^21: cube is < 2^63 */
    }
    if (n >= 4) {
        hi = 55109ULL;
    }
    if (n >= 8) {
        hi = 234ULL;
    }
    if (n >= 16) {
        hi = 16ULL;
    }
    while (lo < hi) {
        uint64_t mid = lo + (hi - lo + 1) / 2;
        u128 acc = u128_from_u64(1);
        int over = 0;
        for (i = 0; i < n; i++) {
            acc = u128_mulw(acc, mid);
            if (acc.hi != 0) {
                over = 1;
                break;
            }
        }
        if (over || acc.lo > v) {
            hi = mid - 1;
        } else {
            lo = mid;
        }
    }
    {
        u128 acc = u128_from_u64(1);
        for (i = 0; i < n; i++) {
            acc = u128_mulw(acc, lo);
        }
        *root = lo;
        return acc.hi == 0 && acc.lo == v;
    }
}

/* ------------------------------------------------------------------------ */
/* arithmetic                                                               */
/* ------------------------------------------------------------------------ */

int ex_neg(ex_t *r, const ex_t *x)
{
    if (x->kind == EX_NONE) {
        ex_none(r);
        return -1;
    }
    *r = *x;
    r->a = -x->a;
    r->b = -x->b;
    return 0;
}

/* scale numerator parts by m/n (n != 0): used for rational factors */
static int scale(ex_t *r, const ex_t *x, int64_t m, int64_t n)
{
    ex_t t = *x;

    if (n == 0 || m == INT64_MIN || n == INT64_MIN) {
        return -1;
    }
    if (mul64(t.a, m, &t.a) != 0 || mul64(t.b, m, &t.b) != 0 || mul64(t.d, n, &t.d) != 0) {
        return -1;
    }
    normalize(&t);
    *r = t;
    return 0;
}

int ex_add(ex_t *r, const ex_t *x, const ex_t *y)
{
    ex_t t;
    uint64_t g, lcm_hi;
    int64_t D, mx, my;
    int64_t a1, a2, b1, b2;

    if (x->kind == EX_NONE || y->kind == EX_NONE) {
        goto fail;
    }
    if (x->kind == EX_PI || y->kind == EX_PI) {
        if (x->kind == EX_PI && y->kind == EX_PI) {
            t.kind = EX_PI;
        } else if (x->kind == EX_QS && x->a == 0 && x->b == 0) {
            *r = *y;
            return 0;
        } else if (y->kind == EX_QS && y->a == 0 && y->b == 0) {
            *r = *x;
            return 0;
        } else {
            goto fail;
        }
    } else {
        if (x->b != 0 && y->b != 0 && x->s != y->s) {
            goto fail;
        }
        t.kind = EX_QS;
    }
    g = gcd64((uint64_t)x->d, (uint64_t)y->d);
    lcm_hi = (uint64_t)x->d / g;
    if (mul64((int64_t)lcm_hi, y->d, &D) != 0) {
        goto fail;
    }
    mx = D / x->d;
    my = D / y->d;
    if (mul64(x->a, mx, &a1) != 0 || mul64(y->a, my, &a2) != 0 ||
        mul64(x->b, mx, &b1) != 0 || mul64(y->b, my, &b2) != 0 ||
        add64(a1, a2, &t.a) != 0 || add64(b1, b2, &t.b) != 0) {
        goto fail;
    }
    t.d = D;
    t.s = x->b != 0 ? x->s : y->s;
    normalize(&t);
    *r = t;
    return 0;
fail:
    ex_none(r);
    return -1;
}

int ex_sub(ex_t *r, const ex_t *x, const ex_t *y)
{
    ex_t ny;

    if (ex_neg(&ny, y) != 0) {
        ex_none(r);
        return -1;
    }
    return ex_add(r, x, &ny);
}

int ex_mul(ex_t *r, const ex_t *x, const ex_t *y)
{
    ex_t t;
    int64_t a, b, c, e, f;

    if (x->kind == EX_NONE || y->kind == EX_NONE) {
        goto fail;
    }
    if (x->kind == EX_PI || y->kind == EX_PI) {
        const ex_t *p = x->kind == EX_PI ? x : y;
        const ex_t *q = x->kind == EX_PI ? y : x;
        if (q->kind != EX_QS || q->b != 0) {
            goto fail;       /* pi*pi or pi*surd */
        }
        t = *p;
        if (mul64(t.a, q->a, &t.a) != 0 || mul64(t.d, q->d, &t.d) != 0) {
            goto fail;
        }
        normalize(&t);
        *r = t;
        return 0;
    }
    /* (a1 + b1 sqrt s1)(a2 + b2 sqrt s2) / (d1 d2) */
    if (mul64(x->d, y->d, &t.d) != 0) {
        goto fail;
    }
    t.kind = EX_QS;
    if (x->b == 0 || y->b == 0 || x->s == y->s) {
        int32_t s = x->b != 0 ? x->s : y->s;
        int64_t ss;

        t.s = s;
        if (x->b == 0 && y->b == 0) {
            if (mul64(x->a, y->a, &t.a) != 0) {
                goto fail;
            }
            t.b = 0;
        } else if (x->b == 0 || y->b == 0) {
            const ex_t *p = x->b == 0 ? x : y;   /* rational */
            const ex_t *q = x->b == 0 ? y : x;
            if (mul64(p->a, q->a, &t.a) != 0 || mul64(p->a, q->b, &t.b) != 0) {
                goto fail;
            }
        } else {
            /* same radicand: a = a1a2 + b1b2 s ; b = a1b2 + a2b1 */
            ss = s;
            if (mul64(x->a, y->a, &a) != 0 || mul64(x->b, y->b, &b) != 0 ||
                mul64(b, ss, &b) != 0 || add64(a, b, &t.a) != 0 ||
                mul64(x->a, y->b, &c) != 0 || mul64(y->a, x->b, &e) != 0 ||
                add64(c, e, &t.b) != 0) {
                goto fail;
            }
        }
    } else {
        /* different radicands: only pure surds multiply to a single surd */
        uint64_t k, sq;
        if (x->a != 0 || y->a != 0) {
            goto fail;
        }
        if (ex_sqrt_split((uint64_t)x->s * (uint64_t)y->s, &k, &sq) != 0) {
            goto fail;
        }
        if (mul64(x->b, y->b, &f) != 0 || mul64(f, (int64_t)k, &f) != 0) {
            goto fail;
        }
        t.a = 0;
        t.b = f;
        t.s = (int32_t)sq;
        if (sq == 1) {
            t.a = f;
            t.b = 0;
        }
    }
    normalize(&t);
    *r = t;
    return 0;
fail:
    ex_none(r);
    return -1;
}

int ex_div(ex_t *r, const ex_t *x, const ex_t *y)
{
    ex_t t, conj;
    int64_t q, a2, b2s, den;

    if (x->kind == EX_NONE || y->kind == EX_NONE) {
        goto fail;
    }
    if (y->a == 0 && y->b == 0) {
        goto fail;
    }
    if (y->kind == EX_PI) {
        if (x->kind != EX_PI) {
            goto fail;       /* n / pi */
        }
        /* (a1/d1) / (a2/d2) = a1 d2 / (d1 a2) */
        {
            int64_t n1, d1;
            if (mul64(x->a, y->d, &n1) != 0 || mul64(x->d, y->a, &d1) != 0) {
                goto fail;
            }
            return ex_rat(r, n1, d1) == 0 ? 0 : -1;
        }
    }
    if (y->b == 0) {
        /* divide by a rational a2/d2: multiply by d2/a2 */
        if (scale(&t, x, y->d, y->a) != 0) {
            goto fail;
        }
        *r = t;
        return 0;
    }
    /* irrational divisor: multiply by the conjugate, divide by the rational norm */
    if (x->kind == EX_PI) {
        goto fail;
    }
    conj = *y;
    conj.b = -conj.b;
    if (ex_mul(&t, x, &conj) != 0) {
        goto fail;
    }
    if (mul64(y->a, y->a, &a2) != 0 || mul64(y->b, y->b, &b2s) != 0 ||
        mul64(b2s, (int64_t)y->s, &b2s) != 0 || add64(a2, -b2s, &den) != 0 || den == 0) {
        goto fail;
    }
    if (mul64(y->d, y->d, &q) != 0) {
        goto fail;
    }
    /* x/y = x*d2/(a2 + b2 sqrt s) = [x*(a2 - b2 sqrt s)/d2] * d2^2 / (a2^2 - b2^2 s) */
    if (scale(&t, &t, q, den) != 0) {
        goto fail;
    }
    *r = t;
    return 0;
fail:
    ex_none(r);
    return -1;
}

int ex_sqrt(ex_t *r, const ex_t *x)
{
    uint64_t n, k, s;
    ex_t t;

    if (!ex_is_rational(x) || x->a < 0) {
        goto fail;
    }
    if (x->a == 0) {
        ex_int(r, 0);
        return 0;
    }
    {
        u128 p = u128_mul_u64((uint64_t)x->a, (uint64_t)x->d);
        if (p.hi != 0 || p.lo > EX_SQRT_LIMIT) {
            goto fail;
        }
        n = p.lo;
    }
    if (ex_sqrt_split(n, &k, &s) != 0) {
        goto fail;
    }
    /* sqrt(a/d) = sqrt(a d)/d = k sqrt(s) / d */
    t.kind = EX_QS;
    t.d = x->d;
    if (s == 1) {
        t.a = (int64_t)k;
        t.b = 0;
        t.s = 1;
    } else {
        t.a = 0;
        t.b = (int64_t)k;
        t.s = (int32_t)s;
    }
    normalize(&t);
    *r = t;
    return 0;
fail:
    ex_none(r);
    return -1;
}

int ex_powi(ex_t *r, const ex_t *x, int64_t n)
{
    ex_t acc, base, one;
    int64_t k = n < 0 ? -n : n;

    if (x->kind == EX_NONE || k > 64) {
        goto fail;
    }
    if (n == 0) {
        ex_int(r, 1);
        return 0;
    }
    if (x->kind == EX_PI) {
        if (n == 1) {
            *r = *x;
            return 0;
        }
        goto fail;
    }
    ex_int(&acc, 1);
    base = *x;
    while (k) {
        if (k & 1) {
            if (ex_mul(&acc, &acc, &base) != 0) {
                goto fail;
            }
        }
        k >>= 1;
        if (k && ex_mul(&base, &base, &base) != 0) {
            goto fail;
        }
    }
    if (n < 0) {
        ex_int(&one, 1);
        return ex_div(r, &one, &acc);
    }
    *r = acc;
    return 0;
fail:
    ex_none(r);
    return -1;
}

int ex_iroot(ex_t *r, const ex_t *x, int64_t n)
{
    uint64_t ra, rd;
    int neg = 0;

    if (!ex_is_rational(x) || n < 2 || n > 62) {
        goto fail;
    }
    if (x->a < 0) {
        if ((n & 1) == 0) {
            goto fail;
        }
        neg = 1;
    }
    if (!iroot_exact(umag(x->a), n, &ra) || !iroot_exact((uint64_t)x->d, n, &rd) || rd == 0) {
        goto fail;
    }
    return ex_rat(r, neg ? -(int64_t)ra : (int64_t)ra, (int64_t)rd);
fail:
    ex_none(r);
    return -1;
}

/* ------------------------------------------------------------------------ */
/* special-angle trigonometry                                               */
/* ------------------------------------------------------------------------ */

/* Angle in whole degrees (0..359) from an exact argument, or -1. */
static int angle_to_degrees(const ex_t *arg, int mode, int64_t *deg)
{
    int64_t n, d, v;

    if (arg->kind == EX_NONE) {
        return -1;
    }
    if (mode == 1) {                     /* radians: need (a/d) pi */
        if (arg->kind != EX_PI) {
            return -1;
        }
        n = arg->a;
        d = arg->d;
        if (mul64(n, 180, &n) != 0) {
            return -1;
        }
    } else {
        if (!ex_is_rational(arg)) {
            return -1;
        }
        n = arg->a;
        d = arg->d;
        if (mode == 2 && mul64(n, 9, &n) != 0) {
            return -1;
        }
        if (mode == 2 && mul64(d, 10, &d) != 0) {
            return -1;
        }
    }
    if (d == 0 || n % d != 0) {
        return -1;
    }
    v = n / d;
    v %= 360;
    if (v < 0) {
        v += 360;
    }
    *deg = v;
    return 0;
}

/* sin of a multiple of 30 or 45 degrees; -1 if not special */
static int sin_deg(int64_t deg, ex_t *out)
{
    int neg = 0;
    int64_t a = deg;

    if (a % 30 != 0 && a % 45 != 0) {
        return -1;
    }
    if (a >= 180) {
        a -= 180;
        neg = 1;
    }
    if (a > 90) {
        a = 180 - a;
    }
    switch (a) {
    case 0:  ex_int(out, 0); break;
    case 30: ex_rat(out, 1, 2); break;
    case 45: out->kind = EX_QS; out->a = 0; out->b = 1; out->d = 2; out->s = 2; break;
    case 60: out->kind = EX_QS; out->a = 0; out->b = 1; out->d = 2; out->s = 3; break;
    case 90: ex_int(out, 1); break;
    default: return -1;
    }
    if (neg) {
        out->a = -out->a;
        out->b = -out->b;
    }
    return 0;
}

int ex_trig(int fn, const ex_t *arg, int mode, ex_t *out)
{
    int64_t deg;
    ex_t s, c;

    if (angle_to_degrees(arg, mode, &deg) != 0) {
        ex_none(out);
        return -1;
    }
    if (fn == 0) {
        return sin_deg(deg, out) == 0 ? 0 : (ex_none(out), -1);
    }
    if (fn == 1) {
        return sin_deg((deg + 90) % 360, out) == 0 ? 0 : (ex_none(out), -1);
    }
    if (sin_deg(deg, &s) != 0 || sin_deg((deg + 90) % 360, &c) != 0) {
        ex_none(out);
        return -1;
    }
    if (c.a == 0 && c.b == 0) {
        ex_none(out);
        return -2;
    }
    return ex_div(out, &s, &c) == 0 ? 0 : -1;
}

int ex_deg_to_mode(const ex_t *deg, int mode, ex_t *out)
{
    if (!ex_is_rational(deg)) {
        ex_none(out);
        return -1;
    }
    if (mode == 0) {
        *out = *deg;
        return 0;
    }
    if (mode == 1) {                     /* deg * pi / 180 */
        int64_t d;
        if (mul64(deg->d, 180, &d) != 0) {
            ex_none(out);
            return -1;
        }
        ex_pi(out, deg->a, d);
        return 0;
    }
    /* grad = deg * 10 / 9 */
    {
        int64_t n, d;
        if (mul64(deg->a, 10, &n) != 0 || mul64(deg->d, 9, &d) != 0) {
            ex_none(out);
            return -1;
        }
        return ex_rat(out, n, d);
    }
}

int ex_itrig(int fn, const ex_t *arg, int mode, ex_t *out)
{
    ex_t cand, deg;
    int64_t d = 0;
    int found = 0;
    int k;
    /* table of (sin value, degrees) for 0..90 */
    static const int64_t degs[5] = { 0, 30, 45, 60, 90 };
    ex_t x = *arg;
    int neg = 0;

    if (arg->kind != EX_QS) {
        ex_none(out);
        return -1;
    }
    if (ex_sign(&x) < 0) {
        neg = 1;
        ex_neg(&x, &x);
    }

    if (fn == 2) {                       /* atan: x in {0, 1/sqrt3, 1, sqrt3} */
        static const int64_t adeg[4] = { 0, 30, 45, 60 };
        for (k = 0; k < 4 && !found; k++) {
            ex_t cs, ss, t;
            if (sin_deg(adeg[k], &ss) != 0 || sin_deg(adeg[k] + 90, &cs) != 0) {
                continue;
            }
            if (cs.a == 0 && cs.b == 0) {
                continue;
            }
            if (ex_div(&t, &ss, &cs) == 0 && ex_equal(&t, &x)) {
                d = adeg[k];
                found = 1;
            }
        }
    } else {                             /* asin / acos on [0,1] */
        for (k = 0; k < 5 && !found; k++) {
            if (sin_deg(degs[k], &cand) == 0 && ex_equal(&cand, &x)) {
                d = degs[k];
                found = 1;
            }
        }
        if (found && fn == 1) {          /* acos(x) = 90 - asin(x), sign handled below */
            d = 90 - d;
        }
    }
    if (!found) {
        ex_none(out);
        return -1;
    }
    if (neg) {
        d = (fn == 1) ? 180 - d : -d;    /* acos(-x) = 180 - acos(x) */
    }
    ex_int(&deg, d);
    return ex_deg_to_mode(&deg, mode, out);
}

/* ------------------------------------------------------------------------ */
/* conversion and formatting                                                */
/* ------------------------------------------------------------------------ */

void ex_to_xn(xnum_t *r, const ex_t *x)
{
    xnum_t a, b, s, d;

    if (x->kind == EX_NONE) {
        xn_zero(r);
        return;
    }
    xn_from_i64(&a, x->a);
    if (x->kind == EX_PI) {
        xn_mul(&a, &a, &XC_PI);
    } else if (x->b != 0) {
        xn_from_i64(&b, x->b);
        xn_from_i64(&s, x->s);
        xn_sqrt(&s, &s);
        xn_mul(&b, &b, &s);
        xn_add(&a, &a, &b);
    }
    if (x->d != 1) {
        xn_from_i64(&d, x->d);
        xn_div(&a, &a, &d);
    }
    *r = a;
}

dec_status ex_to_dec(dec_t *r, const ex_t *x)
{
    xnum_t t;

    if (x->kind == EX_NONE) {
        dec_zero(r);
        return DEC_DOMAIN;
    }
    ex_to_xn(&t, x);
    return xn_to_dec(r, &t);
}

static void put_u64(char *buf, size_t *pos, size_t cap, uint64_t v)
{
    char tmp[24];
    int n = 0;

    if (v == 0) {
        tmp[n++] = '0';
    }
    while (v > 0 && n < 22) {
        tmp[n++] = (char)('0' + (int)(v % 10u));
        v /= 10u;
    }
    while (n > 0 && *pos + 1 < cap) {
        buf[(*pos)++] = tmp[--n];
    }
}

static void put_str(char *buf, size_t *pos, size_t cap, const char *s)
{
    while (*s && *pos + 1 < cap) {
        buf[(*pos)++] = *s++;
    }
}

void ex_format(const ex_t *x, int ascii, ex_text_t *out)
{
    size_t p = 0;
    size_t cap = sizeof out->num;
    const char *surd = ascii ? "sqrt(" : "\xE2\x88\x9A";   /* √ */
    const char *pi = ascii ? "pi" : "\xCF\x80";            /* π */
    uint64_t am = umag(x->a);
    uint64_t bm = umag(x->b);
    int neg;

    out->neg = 0;
    out->num[0] = '\0';
    out->den[0] = '\0';
    if (x->kind == EX_NONE) {
        return;
    }
    if (x->kind == EX_PI) {
        neg = x->a < 0;
        out->neg = (char)neg;
        if (am != 1) {
            put_u64(out->num, &p, cap, am);
        }
        put_str(out->num, &p, cap, pi);
    } else if (x->b == 0) {
        neg = x->a < 0;
        out->neg = (char)neg;
        put_u64(out->num, &p, cap, am);
    } else if (x->a == 0) {
        neg = x->b < 0;
        out->neg = (char)neg;
        if (bm != 1) {
            put_u64(out->num, &p, cap, bm);
        }
        put_str(out->num, &p, cap, surd);
        put_u64(out->num, &p, cap, (uint64_t)x->s);
        if (ascii) {
            put_str(out->num, &p, cap, ")");
        }
    } else {
        /* two terms: the sign of each term stays inside the numerator */
        out->neg = 0;
        if (x->a < 0) {
            put_str(out->num, &p, cap, "-");
        }
        put_u64(out->num, &p, cap, am);
        put_str(out->num, &p, cap, x->b < 0 ? "-" : "+");
        if (bm != 1) {
            put_u64(out->num, &p, cap, bm);
        }
        put_str(out->num, &p, cap, surd);
        put_u64(out->num, &p, cap, (uint64_t)x->s);
        if (ascii) {
            put_str(out->num, &p, cap, ")");
        }
    }
    out->num[p] = '\0';
    if (x->d != 1) {
        size_t q = 0;
        put_u64(out->den, &q, sizeof out->den, (uint64_t)x->d);
        out->den[q] = '\0';
    }
}
