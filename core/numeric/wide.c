#include "numeric/wide.h"

const uint64_t u64_pow10[20] = {
    1ULL,
    10ULL,
    100ULL,
    1000ULL,
    10000ULL,
    100000ULL,
    1000000ULL,
    10000000ULL,
    100000000ULL,
    1000000000ULL,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL,
    10000000000000000000ULL
};

u128 u128_mul_u64(uint64_t a, uint64_t b)
{
    const uint64_t mask = 0xFFFFFFFFULL;
    uint64_t a0 = a & mask;
    uint64_t a1 = a >> 32;
    uint64_t b0 = b & mask;
    uint64_t b1 = b >> 32;
    uint64_t p00 = a0 * b0;
    uint64_t p01 = a0 * b1;
    uint64_t p10 = a1 * b0;
    uint64_t p11 = a1 * b1;
    uint64_t mid = (p00 >> 32) + (p01 & mask) + (p10 & mask);
    u128 r;

    r.lo = (p00 & mask) | (mid << 32);
    r.hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
    return r;
}

u128 u128_mulw(u128 a, uint64_t b)
{
    u128 r = u128_mul_u64(a.lo, b);
    r.hi += a.hi * b;
    return r;
}

uint32_t u128_divmod_u32(u128 *a, uint32_t d)
{
    uint32_t w[4];
    uint64_t rem = 0;
    int i;

    w[0] = (uint32_t)(a->hi >> 32);
    w[1] = (uint32_t)(a->hi & 0xFFFFFFFFULL);
    w[2] = (uint32_t)(a->lo >> 32);
    w[3] = (uint32_t)(a->lo & 0xFFFFFFFFULL);

    for (i = 0; i < 4; i++) {
        uint64_t cur = (rem << 32) | w[i];
        w[i] = (uint32_t)(cur / d);
        rem = cur % d;
    }

    a->hi = ((uint64_t)w[0] << 32) | w[1];
    a->lo = ((uint64_t)w[2] << 32) | w[3];
    return (uint32_t)rem;
}

uint64_t u128_isqrt(u128 n, int *inexact)
{
    /* Classic bit-by-bit integer square root. */
    u128 res = u128_from_u64(0);
    u128 bit;
    bit.hi = 1ULL << 62; /* 2^126, the highest power of four in 128 bits */
    bit.lo = 0;

    while (u128_cmp(bit, n) > 0) {
        bit = u128_shr(bit, 2);
    }
    while (!u128_is_zero(bit)) {
        u128 t = u128_add(res, bit);
        if (u128_cmp(n, t) >= 0) {
            n = u128_sub(n, t);
            res = u128_add(u128_shr(res, 1), bit);
        } else {
            res = u128_shr(res, 1);
        }
        bit = u128_shr(bit, 2);
    }

    *inexact = !u128_is_zero(n); /* n now holds the remainder */
    return res.lo;
}

int u64_ndigits(uint64_t x)
{
    int n = 1;
    while (n < 20 && x >= u64_pow10[n]) {
        n++;
    }
    return n;
}

int u128_ndigits(u128 n)
{
    int nd = 0;
    while (n.hi != 0) {
        (void)u128_divmod_u32(&n, 1000000000u);
        nd += 9;
    }
    return nd + u64_ndigits(n.lo);
}
