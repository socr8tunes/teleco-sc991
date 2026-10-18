/*
 * Minimal portable 128-bit unsigned integer helpers.
 *
 * Deliberately avoids __int128 so the exact same code builds with any C99
 * compiler, including 32-bit embedded toolchains.
 */
#ifndef TELECO_NUMERIC_WIDE_H
#define TELECO_NUMERIC_WIDE_H

#include <stdint.h>

typedef struct u128 {
    uint64_t hi;
    uint64_t lo;
} u128;

/* 10^0 .. 10^19 (10^19 is the largest power of ten that fits in 64 bits). */
extern const uint64_t u64_pow10[20];

static inline u128 u128_from_u64(uint64_t x)
{
    u128 r;
    r.hi = 0;
    r.lo = x;
    return r;
}

static inline int u128_is_zero(u128 a)
{
    return (a.hi | a.lo) == 0;
}

static inline int u128_cmp(u128 a, u128 b)
{
    if (a.hi != b.hi) {
        return a.hi < b.hi ? -1 : 1;
    }
    if (a.lo != b.lo) {
        return a.lo < b.lo ? -1 : 1;
    }
    return 0;
}

static inline u128 u128_add(u128 a, u128 b)
{
    u128 r;
    r.lo = a.lo + b.lo;
    r.hi = a.hi + b.hi + (r.lo < a.lo ? 1u : 0u);
    return r;
}

static inline u128 u128_sub(u128 a, u128 b)
{
    u128 r;
    r.lo = a.lo - b.lo;
    r.hi = a.hi - b.hi - (a.lo < b.lo ? 1u : 0u);
    return r;
}

/* Logical shift right by n bits, valid for 1 <= n <= 63. */
static inline u128 u128_shr(u128 a, unsigned n)
{
    u128 r;
    r.lo = (a.lo >> n) | (a.hi << (64u - n));
    r.hi = a.hi >> n;
    return r;
}

/* Full 64 x 64 -> 128 bit product. */
u128 u128_mul_u64(uint64_t a, uint64_t b);

/* 128 x 64 -> 128 product. The caller guarantees the result fits in 128 bits. */
u128 u128_mulw(u128 a, uint64_t b);

/* In-place a /= d, returns a % d. Requires 0 < d < 2^32. */
uint32_t u128_divmod_u32(u128 *a, uint32_t d);

/*
 * floor(sqrt(n)). *inexact is set to non-zero when n is not a perfect square
 * (that is, when the true root lies strictly between the result and result+1).
 */
uint64_t u128_isqrt(u128 n, int *inexact);

/* Number of decimal digits (x == 0 counts as 1 digit). */
int u64_ndigits(uint64_t x);
int u128_ndigits(u128 n);

#endif /* TELECO_NUMERIC_WIDE_H */
