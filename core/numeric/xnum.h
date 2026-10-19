/*
 * xnum: extended-precision decimal-ish float used INSIDE the function library
 * (exp, ln, trig, ...) so that results can be correctly rounded to the 16
 * digits of dec_t.
 *
 * Representation: 6 limbs in base 10^9, most significant first.
 *     value = (-1)^neg * sum_i  l[i] * B^(e - 1 - i),   B = 10^9
 * Normalised: l[0] != 0 unless the value is zero (then e == 0, neg == 0).
 * Precision: 45 to 54 significant digits (depends on the size of l[0]).
 *
 * No heap, no floating point, portable C99.
 */
#ifndef TELECO_NUMERIC_XNUM_H
#define TELECO_NUMERIC_XNUM_H

#include <stdint.h>
#include "numeric/decimal.h"

#define XN_LIMBS 6
#define XN_BASE  1000000000u
#define XN_EMAX  (1 << 24)   /* exponents are clamped to +-XN_EMAX (=> overflow / underflow later) */

typedef struct xnum {
    uint32_t l[XN_LIMBS];
    int32_t  e;
    uint8_t  neg;
} xnum_t;

/* mode argument of xn_to_i64 */
#define XN_TRUNC 0
#define XN_ROUND 1   /* nearest, ties away from zero */
#define XN_FLOOR 2

void xn_zero(xnum_t *r);
int  xn_is_zero(const xnum_t *a);
int  xn_sign(const xnum_t *a);                         /* -1, 0, 1 */
int  xn_cmp(const xnum_t *a, const xnum_t *b);
int  xn_cmp_abs(const xnum_t *a, const xnum_t *b);
void xn_neg(xnum_t *r, const xnum_t *a);
void xn_abs(xnum_t *r, const xnum_t *a);

void xn_from_u64(xnum_t *r, uint64_t v);
void xn_from_i64(xnum_t *r, int64_t v);
void xn_from_dec(xnum_t *r, const dec_t *d);
/* Round to 16 digits (half-even). Overflow/underflow reported like dec_pack_internal. */
dec_status xn_to_dec(dec_t *r, const xnum_t *a);

/* Integer conversion. Returns 0 on success, -1 if |a| is too large for int64. */
int  xn_to_i64(const xnum_t *a, int mode, int64_t *out);
int  xn_is_int(const xnum_t *a);

void xn_add(xnum_t *r, const xnum_t *a, const xnum_t *b);
void xn_sub(xnum_t *r, const xnum_t *a, const xnum_t *b);
void xn_mul(xnum_t *r, const xnum_t *a, const xnum_t *b);
void xn_div(xnum_t *r, const xnum_t *a, const xnum_t *b);   /* b must be non-zero */
void xn_sqrt(xnum_t *r, const xnum_t *a);                    /* a must be >= 0 */
void xn_mul_u32(xnum_t *r, const xnum_t *a, uint32_t m);
void xn_div_u32(xnum_t *r, const xnum_t *a, uint32_t d);     /* d != 0 */
void xn_pow10(xnum_t *r, int32_t k);
void xn_powi(xnum_t *r, const xnum_t *a, int64_t n);         /* a != 0 or n > 0 */

/* floor(log10(|a|)); a must be non-zero. */
int32_t xn_dec_exponent(const xnum_t *a);

#endif /* TELECO_NUMERIC_XNUM_H */
