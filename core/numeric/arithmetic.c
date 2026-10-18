#include "numeric/decimal.h"
#include "numeric/decimal_internal.h"

/*
 * Addition and subtraction.
 *
 * The larger-magnitude operand x is scaled by 10^18 into a 128-bit integer X.
 * The smaller operand y is aligned to the same scale. If y lies so far below
 * x that it cannot reach the 18 guard digits (exponent gap > 18), it only
 * contributes a sticky fraction, which is all rounding needs to know.
 */
static dec_status addsub(dec_t *r, const dec_t *a, const dec_t *b, int flip_b)
{
    dec_t x = *a;
    dec_t y = *b;
    int cmp_mag;
    int32_t d;
    int sticky = 0;
    u128 big;
    u128 small;
    u128 n;

    if (y.m == 0) {
        *r = x;
        return DEC_OK;
    }
    y.neg = (uint8_t)(y.neg ^ (flip_b ? 1u : 0u));
    if (x.m == 0) {
        *r = y;
        return DEC_OK;
    }

    if (x.exp != y.exp) {
        cmp_mag = x.exp > y.exp ? 1 : -1;
    } else if (x.m != y.m) {
        cmp_mag = x.m > y.m ? 1 : -1;
    } else {
        cmp_mag = 0;
    }

    if (cmp_mag == 0 && x.neg != y.neg) {
        dec_zero(r); /* exact cancellation */
        return DEC_OK;
    }
    if (cmp_mag < 0) {
        dec_t t = x;
        x = y;
        y = t;
    }

    d = (int32_t)x.exp - (int32_t)y.exp; /* >= 0 */
    big = u128_mulw(u128_from_u64(x.m), u64_pow10[18]);
    if (d <= 18) {
        small = u128_mulw(u128_from_u64(y.m), u64_pow10[18 - d]);
    } else {
        small = u128_from_u64(0);
        sticky = 1;
    }

    if (x.neg == y.neg) {
        n = u128_add(big, small);
    } else {
        n = u128_sub(big, small);
        if (sticky) {
            /* true value is big - small - epsilon = (n - 1) + (1 - epsilon) */
            n = u128_sub(n, u128_from_u64(1));
        }
    }

    return dec_pack_internal(r, x.neg, n, sticky, (int32_t)x.exp - (DEC_DIGITS - 1) - 18);
}

dec_status dec_add(dec_t *r, const dec_t *a, const dec_t *b)
{
    return addsub(r, a, b, 0);
}

dec_status dec_sub(dec_t *r, const dec_t *a, const dec_t *b)
{
    return addsub(r, a, b, 1);
}

dec_status dec_mul(dec_t *r, const dec_t *a, const dec_t *b)
{
    u128 n;
    int neg;
    int32_t e;

    if (a->m == 0 || b->m == 0) {
        dec_zero(r);
        return DEC_OK;
    }

    n = u128_mul_u64(a->m, b->m); /* exact, 31 or 32 digits */
    neg = a->neg ^ b->neg;
    e = ((int32_t)a->exp - (DEC_DIGITS - 1)) + ((int32_t)b->exp - (DEC_DIGITS - 1));
    return dec_pack_internal(r, neg, n, 0, e);
}

/*
 * Division: q = floor(ma * 10^17 / mb), computed by schoolbook long division
 * in chunks of up to 3 decimal digits so only 64-bit arithmetic is needed
 * (rem < mb < 10^16, and rem * 10^3 < 2^64). q has 17 or 18 digits, giving
 * at least one guard digit; a non-zero final remainder is the sticky bit.
 */
dec_status dec_div(dec_t *r, const dec_t *a, const dec_t *b)
{
    static const unsigned chunk[6] = { 3, 3, 3, 3, 3, 2 };
    uint64_t ma = a->m;
    uint64_t mb = b->m;
    int neg = a->neg ^ b->neg;
    int32_t e;
    uint64_t q;
    uint64_t rem;
    int i;

    if (mb == 0) {
        dec_zero(r);
        return ma == 0 ? DEC_DOMAIN : DEC_DIV_BY_ZERO;
    }
    if (ma == 0) {
        dec_zero(r);
        return DEC_OK;
    }

    e = (int32_t)a->exp - (int32_t)b->exp - 17;

    q = ma / mb;
    rem = ma % mb;
    for (i = 0; i < 6; i++) {
        uint64_t f = u64_pow10[chunk[i]];
        rem *= f;
        q = q * f + rem / mb;
        rem %= mb;
    }

    return dec_pack_internal(r, neg, u128_from_u64(q), rem != 0, e);
}

/*
 * Square root: make the exponent of the last digit even, scale the significand
 * by 10^18 and take the exact integer square root. The root has 17 or 18
 * digits; a non-perfect square leaves a remainder, which is the sticky bit.
 */
dec_status dec_sqrt(dec_t *r, const dec_t *a)
{
    uint64_t m = a->m;
    int32_t e;
    int inexact;
    uint64_t root;
    u128 n;

    if (m == 0) {
        dec_zero(r);
        return DEC_OK;
    }
    if (a->neg) {
        dec_zero(r);
        return DEC_DOMAIN;
    }

    e = (int32_t)a->exp - (DEC_DIGITS - 1);
    if (e % 2 != 0) {
        m *= 10u;
        e -= 1;
    }

    n = u128_mulw(u128_from_u64(m), u64_pow10[18]);
    root = u128_isqrt(n, &inexact);
    return dec_pack_internal(r, 0, u128_from_u64(root), inexact, e / 2 - 9);
}
