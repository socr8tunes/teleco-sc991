#include "numeric/xnum.h"
#include "numeric/decimal_internal.h"

#define B XN_BASE

/* ------------------------------------------------------------------------ */
/* helpers                                                                  */
/* ------------------------------------------------------------------------ */

static int32_t clamp_e(int32_t e)
{
    if (e > XN_EMAX) {
        return XN_EMAX;
    }
    if (e < -XN_EMAX) {
        return -XN_EMAX;
    }
    return e;
}

/*
 * Build a normalised xnum from the fraction 0.p[0] p[1] ... p[n-1] (base B)
 * times B^e. p[] may have leading zero limbs and up to 16 limbs. The value is
 * rounded to 4 limbs, half up, using limb 4.
 */
static void norm(xnum_t *r, int neg, const uint32_t *pin, int n, int32_t e)
{
    uint32_t p[24];
    int z = 0;
    int i;

    for (i = 0; i < n; i++) {
        p[i] = pin[i];
    }
    while (z < n && p[z] == 0) {
        z++;
    }
    if (z == n) {
        xn_zero(r);
        return;
    }
    for (i = 0; i < n - z; i++) {
        p[i] = p[i + z];
    }
    n -= z;
    e -= z;

    if (n > XN_LIMBS && p[XN_LIMBS] >= B / 2) {
        i = XN_LIMBS - 1;
        for (;;) {
            p[i]++;
            if (p[i] < B) {
                break;
            }
            p[i] = 0;
            if (i == 0) {
                /* carried out of the leading limb: value is now exactly B^e */
                p[0] = 1;
                e++;
                break;
            }
            i--;
        }
    }

    for (i = 0; i < XN_LIMBS; i++) {
        r->l[i] = (i < n) ? p[i] : 0u;
    }
    r->e = clamp_e(e);
    r->neg = (uint8_t)(neg != 0);
}

/* u128 -> xnum: value = n * B^k */
static void from_u128_scaled(xnum_t *r, int neg, u128 n, int32_t k)
{
    uint32_t c[5];
    uint32_t p[5];
    int t = 0;
    int i;

    while (!u128_is_zero(n) && t < 5) {
        c[t++] = u128_divmod_u32(&n, B);
    }
    if (t == 0) {
        xn_zero(r);
        return;
    }
    for (i = 0; i < t; i++) {
        p[i] = c[t - 1 - i];
    }
    norm(r, neg, p, t, (int32_t)t + k);
}

static int32_t floor_div(int32_t a, int32_t b)
{
    int32_t q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) {
        q--;
    }
    return q;
}

/* ------------------------------------------------------------------------ */
/* basics                                                                   */
/* ------------------------------------------------------------------------ */

void xn_zero(xnum_t *r)
{
    int i;
    for (i = 0; i < XN_LIMBS; i++) {
        r->l[i] = 0;
    }
    r->e = 0;
    r->neg = 0;
}

int xn_is_zero(const xnum_t *a)
{
    return a->l[0] == 0;
}

int xn_sign(const xnum_t *a)
{
    if (a->l[0] == 0) {
        return 0;
    }
    return a->neg ? -1 : 1;
}

int xn_cmp_abs(const xnum_t *a, const xnum_t *b)
{
    int i;

    if (a->l[0] == 0 || b->l[0] == 0) {
        if (a->l[0] == b->l[0]) {
            return 0;
        }
        return a->l[0] == 0 ? -1 : 1;
    }
    if (a->e != b->e) {
        return a->e > b->e ? 1 : -1;
    }
    for (i = 0; i < XN_LIMBS; i++) {
        if (a->l[i] != b->l[i]) {
            return a->l[i] > b->l[i] ? 1 : -1;
        }
    }
    return 0;
}

int xn_cmp(const xnum_t *a, const xnum_t *b)
{
    int sa = xn_sign(a);
    int sb = xn_sign(b);

    if (sa != sb) {
        return sa < sb ? -1 : 1;
    }
    if (sa == 0) {
        return 0;
    }
    return sa > 0 ? xn_cmp_abs(a, b) : -xn_cmp_abs(a, b);
}

void xn_neg(xnum_t *r, const xnum_t *a)
{
    *r = *a;
    if (r->l[0] != 0) {
        r->neg = (uint8_t)(r->neg ^ 1u);
    }
}

void xn_abs(xnum_t *r, const xnum_t *a)
{
    *r = *a;
    r->neg = 0;
}

void xn_from_u64(xnum_t *r, uint64_t v)
{
    from_u128_scaled(r, 0, u128_from_u64(v), 0);
}

void xn_from_i64(xnum_t *r, int64_t v)
{
    uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
    from_u128_scaled(r, v < 0, u128_from_u64(mag), 0);
}

void xn_from_dec(xnum_t *r, const dec_t *d)
{
    int32_t p;
    int32_t k;
    int32_t rem;
    u128 n;

    if (d->m == 0) {
        xn_zero(r);
        return;
    }
    p = (int32_t)d->exp - (DEC_DIGITS - 1);   /* decimal exponent of the last digit */
    k = floor_div(p, 9);
    rem = p - 9 * k;                           /* 0..8 */
    n = u128_mul_u64(d->m, u64_pow10[rem]);
    from_u128_scaled(r, d->neg, n, k);
}

/*
 * Values that are exactly representable (for example 20^-23, an exact tie at
 * digit 17) come out of Newton division / series with a residue of a few ulps in
 * the last limbs. Snap anything within a RELATIVE 1e-40 of such a value, so that
 * exact ties round half-even instead of by noise. 16-digit inputs can only create
 * genuine near-ties at >= ~1e-33 relative (e.g. sqrt(100 - 1e-14)), and the
 * arithmetic noise is ~1e-44, so the window sits 7 orders of magnitude from both.
 * Window in units of the last limb: l0 * 1e5 (unit relative size is 1e-45 / l0).
 */
#if XN_LIMBS != 6
#error "xn_to_dec snapping assumes XN_LIMBS == 6"
#endif

dec_status xn_to_dec(dec_t *r, const xnum_t *a)
{
    u128 n;
    uint32_t l[XN_LIMBS];
    int32_t e = a->e;
    int i;
    int sticky;
    uint64_t tail;
    uint64_t window;

    if (a->l[0] == 0) {
        dec_zero(r);
        return DEC_OK;
    }
    if (e > 14) {
        dec_zero(r);
        return DEC_OVERFLOW;
    }
    if (e < -14) {
        dec_zero(r);
        return DEC_UNDERFLOW;
    }
    for (i = 0; i < XN_LIMBS; i++) {
        l[i] = a->l[i];
    }
    tail = (uint64_t)l[4] * B + l[5];
    window = (uint64_t)l[0] * 100000u;
    if (l[3] == 0 && tail <= window) {
        l[4] = 0;
        l[5] = 0;
    } else if (l[3] == B - 1u && (uint64_t)B * B - tail <= window) {
        l[3] = 0;
        l[4] = 0;
        l[5] = 0;
        i = 2;
        for (;;) {
            l[i]++;
            if (l[i] < B) {
                break;
            }
            l[i] = 0;
            if (i == 0) {
                l[0] = 1;
                e++;
                break;
            }
            i--;
        }
    }
    sticky = (l[3] | l[4] | l[5]) != 0;
    n = u128_mul_u64(l[0], (uint64_t)B);
    n = u128_mulw(n, (uint64_t)B);                       /* l0 * B^2 */
    n = u128_add(n, u128_mul_u64(l[1], (uint64_t)B));    /* + l1 * B */
    n = u128_add(n, u128_from_u64(l[2]));
    return dec_pack_internal(r, a->neg, n, sticky, 9 * (e - 3));
}

int xn_to_i64(const xnum_t *a, int mode, int64_t *out)
{
    uint64_t ip = 0;
    int i;
    int32_t pos;
    int up = 0;
    int frac_nonzero = 0;

    if (a->l[0] == 0) {
        *out = 0;
        return 0;
    }
    if (a->e > 2) {
        return -1; /* >= B^2 = 1e18 */
    }
    for (i = 0; i < XN_LIMBS; i++) {
        pos = a->e - 1 - i; /* weight B^pos */
        if (pos >= 0) {
            uint64_t w = (pos == 0) ? 1u : (pos == 1 ? (uint64_t)B : (uint64_t)B * B);
            ip += (uint64_t)a->l[i] * w;
        } else {
            if (pos == -1 && a->l[i] >= B / 2) {
                up = 1;
            }
            if (a->l[i] != 0) {
                frac_nonzero = 1;
            }
        }
    }
    if (ip > ((uint64_t)1 << 62)) {
        return -1;
    }
    if (mode == XN_ROUND && up) {
        ip++;
    } else if (mode == XN_FLOOR && a->neg && frac_nonzero) {
        ip++;
    }
    *out = a->neg ? -(int64_t)ip : (int64_t)ip;
    return 0;
}

int xn_is_int(const xnum_t *a)
{
    int i;

    if (a->l[0] == 0) {
        return 1;
    }
    for (i = 0; i < XN_LIMBS; i++) {
        if (a->e - 1 - i < 0 && a->l[i] != 0) {
            return 0;
        }
    }
    return 1;
}

int32_t xn_dec_exponent(const xnum_t *a)
{
    int nd = u64_ndigits(a->l[0]);
    return 9 * (a->e - 1) + (nd - 1);
}

/* ------------------------------------------------------------------------ */
/* arithmetic                                                               */
/* ------------------------------------------------------------------------ */

#define W (XN_LIMBS + 12)  /* working limbs for addition: 1 carry slot + room for the shifted operand */

void xn_add(xnum_t *r, const xnum_t *a, const xnum_t *b)
{
    const xnum_t *x = a;
    const xnum_t *y = b;
    uint32_t t[W];
    uint32_t ys[W];
    int32_t s;
    int i;

    if (a->l[0] == 0) {
        *r = *b;
        return;
    }
    if (b->l[0] == 0) {
        *r = *a;
        return;
    }
    if (xn_cmp_abs(x, y) < 0) {
        x = b;
        y = a;
    }
    s = x->e - y->e;
    if (s >= 11) {
        *r = *x; /* y is below 10^-99 of x: irrelevant at our precision */
        return;
    }

    for (i = 0; i < W; i++) {
        t[i] = 0;
        ys[i] = 0;
    }
    for (i = 0; i < XN_LIMBS; i++) {
        t[i + 1] = x->l[i];
        if (i + 1 + s < W) {
            ys[i + 1 + s] = y->l[i];
        }
    }

    if (x->neg == y->neg) {
        uint32_t carry = 0;
        for (i = W - 1; i >= 0; i--) {
            uint32_t v = t[i] + ys[i] + carry;
            if (v >= B) {
                v -= B;
                carry = 1;
            } else {
                carry = 0;
            }
            t[i] = v;
        }
    } else {
        uint32_t borrow = 0;
        for (i = W - 1; i >= 0; i--) {
            int64_t v = (int64_t)t[i] - (int64_t)ys[i] - (int64_t)borrow;
            if (v < 0) {
                v += B;
                borrow = 1;
            } else {
                borrow = 0;
            }
            t[i] = (uint32_t)v;
        }
    }
    norm(r, x->neg, t, W, x->e + 1);
}

void xn_sub(xnum_t *r, const xnum_t *a, const xnum_t *b)
{
    xnum_t nb;
    xn_neg(&nb, b);
    xn_add(r, a, &nb);
}

void xn_mul(xnum_t *r, const xnum_t *a, const xnum_t *b)
{
    uint64_t t[2 * XN_LIMBS];
    uint32_t p[2 * XN_LIMBS];
    int i;
    int j;

    if (a->l[0] == 0 || b->l[0] == 0) {
        xn_zero(r);
        return;
    }
    for (i = 0; i < 2 * XN_LIMBS; i++) {
        t[i] = 0;
    }
    for (i = 0; i < XN_LIMBS; i++) {
        for (j = 0; j < XN_LIMBS; j++) {
            t[i + j + 1] += (uint64_t)a->l[i] * b->l[j];
        }
    }
    for (i = 2 * XN_LIMBS - 1; i > 0; i--) {
        t[i - 1] += t[i] / B;
        t[i] %= B;
    }
    for (i = 0; i < 2 * XN_LIMBS; i++) {
        p[i] = (uint32_t)t[i];
    }
    norm(r, a->neg ^ b->neg, p, 2 * XN_LIMBS, a->e + b->e);
}

void xn_mul_u32(xnum_t *r, const xnum_t *a, uint32_t m)
{
    uint32_t p[XN_LIMBS + 2];
    uint64_t carry = 0;
    int i;

    if (a->l[0] == 0 || m == 0) {
        xn_zero(r);
        return;
    }
    /* result limbs: p[0] carry-out slot (up to 2 limbs for m < 2^32), p[1..4] */
    p[0] = 0;
    p[1] = 0;
    for (i = XN_LIMBS - 1; i >= 0; i--) {
        uint64_t v = (uint64_t)a->l[i] * m + carry;
        p[i + 2] = (uint32_t)(v % B);
        carry = v / B;
    }
    p[1] = (uint32_t)(carry % B);
    p[0] = (uint32_t)(carry / B);
    norm(r, a->neg, p, XN_LIMBS + 2, a->e + 2);
}

void xn_div_u32(xnum_t *r, const xnum_t *a, uint32_t d)
{
    uint32_t p[XN_LIMBS + 3];
    uint64_t rem = 0;
    int i;

    if (a->l[0] == 0) {
        xn_zero(r);
        return;
    }
    for (i = 0; i < XN_LIMBS + 3; i++) {
        uint64_t cur = rem * B + (i < XN_LIMBS ? a->l[i] : 0u);
        p[i] = (uint32_t)(cur / d);
        rem = cur % d;
    }
    norm(r, a->neg, p, XN_LIMBS + 3, a->e);
}

void xn_pow10(xnum_t *r, int32_t k)
{
    int32_t q = floor_div(k, 9);
    int32_t rem = k - 9 * q;

    xn_zero(r);
    r->l[0] = (uint32_t)u64_pow10[rem];
    r->e = clamp_e(q + 1);
}

static void xn_const_two(xnum_t *r)
{
    xn_from_u64(r, 2);
}

/* 1/y */
static void xn_recip(xnum_t *r, const xnum_t *y)
{
    xnum_t ys, r0, t, two;
    dec_t d, one, rd;
    int it;
    int32_t ey = y->e;

    ys = *y;
    ys.e = 0;
    ys.neg = 0;
    (void)xn_to_dec(&d, &ys);
    dec_from_i64(&one, 1);
    (void)dec_div(&rd, &one, &d);
    xn_from_dec(&r0, &rd);
    xn_const_two(&two);
    for (it = 0; it < 2; it++) {
        xn_mul(&t, &ys, &r0);
        xn_sub(&t, &two, &t);
        xn_mul(&r0, &r0, &t);
    }
    r0.e = clamp_e(r0.e - ey);
    r0.neg = y->neg;
    *r = r0;
}

void xn_div(xnum_t *r, const xnum_t *a, const xnum_t *b)
{
    xnum_t rc, q, t;

    if (a->l[0] == 0) {
        xn_zero(r);
        return;
    }
    xn_recip(&rc, b);
    xn_mul(&q, a, &rc);
    /* one correction step: q += rc * (a - q*b) */
    xn_mul(&t, &q, b);
    xn_sub(&t, a, &t);
    xn_mul(&t, &t, &rc);
    xn_add(r, &q, &t);
}

void xn_sqrt(xnum_t *r, const xnum_t *a)
{
    xnum_t t, r0, u, v, s, three;
    dec_t d, sd, one, rd;
    int32_t ee;
    int32_t half;
    int it;

    if (a->l[0] == 0) {
        xn_zero(r);
        return;
    }
    ee = a->e;
    half = floor_div(ee, 2);
    t = *a;
    t.e = ee - 2 * half; /* 0 or 1: t in [1/B, B) */
    t.neg = 0;

    (void)xn_to_dec(&d, &t);
    (void)dec_sqrt(&sd, &d);
    dec_from_i64(&one, 1);
    (void)dec_div(&rd, &one, &sd);
    xn_from_dec(&r0, &rd);
    xn_from_u64(&three, 3);

    for (it = 0; it < 2; it++) { /* r <- r (3 - t r^2) / 2 */
        xn_mul(&u, &r0, &r0);
        xn_mul(&u, &u, &t);
        xn_sub(&v, &three, &u);
        xn_div_u32(&v, &v, 2);
        xn_mul(&r0, &r0, &v);
    }
    xn_mul(&s, &t, &r0);          /* s ~ sqrt(t) */
    xn_mul(&u, &s, &s);
    xn_sub(&u, &t, &u);           /* t - s^2 */
    xn_mul(&u, &u, &r0);
    xn_div_u32(&u, &u, 2);
    xn_add(&s, &s, &u);
    s.e = clamp_e(s.e + half);
    *r = s;
}

void xn_powi(xnum_t *r, const xnum_t *a, int64_t n)
{
    xnum_t base, acc;
    uint64_t k;
    int invert = 0;

    if (n == 0) {
        xn_from_u64(r, 1);
        return;
    }
    if (n < 0) {
        invert = 1;
        k = (uint64_t)0 - (uint64_t)n;
    } else {
        k = (uint64_t)n;
    }
    base = *a;
    xn_from_u64(&acc, 1);
    while (k) {
        if (k & 1u) {
            xn_mul(&acc, &acc, &base);
        }
        k >>= 1;
        if (k) {
            xn_mul(&base, &base, &base);
        }
    }
    if (invert) {
        xnum_t one;
        xn_from_u64(&one, 1);
        xn_div(r, &one, &acc);
    } else {
        *r = acc;
    }
}
