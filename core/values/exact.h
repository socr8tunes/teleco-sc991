/*
 * Exact values for natural-display results.
 *
 *   EX_QS : (a + b*sqrt(s)) / d        s squarefree > 1 when b != 0, d > 0
 *           (rationals are the case b == 0)
 *   EX_PI : (a / d) * pi               b == 0
 *   EX_NONE : no exact form known (the decimal value is authoritative)
 *
 * Every operation returns 0 on success and -1 when the result cannot be
 * represented (mixed radicands, pi*pi, 64-bit overflow, radicand too large...).
 * Callers then simply drop to the decimal value. No heap, no floating point.
 */
#ifndef TELECO_VALUES_EXACT_H
#define TELECO_VALUES_EXACT_H

#include <stddef.h>
#include <stdint.h>
#include "numeric/decimal.h"
#include "numeric/xnum.h"

#define EX_NONE 0
#define EX_QS   1
#define EX_PI   2

#define EX_SQRT_LIMIT 10000000000ULL   /* largest radicand we factor (trial division) */

typedef struct ex {
    uint8_t kind;
    int64_t a, b, d;
    int64_t s;   /* squarefree radicand, up to EX_SQRT_LIMIT */
} ex_t;

void ex_none(ex_t *r);
void ex_int(ex_t *r, int64_t n);
int  ex_rat(ex_t *r, int64_t num, int64_t den);   /* den != 0 */
void ex_pi(ex_t *r, int64_t num, int64_t den);    /* (num/den) * pi, den != 0 */

int  ex_valid(const ex_t *x);
int  ex_is_rational(const ex_t *x);               /* kind QS with b == 0 */
int  ex_is_integer(const ex_t *x);
int  ex_equal(const ex_t *x, const ex_t *y);
int  ex_sign(const ex_t *x);                      /* -1, 0, 1 (EX_NONE: 0) */

int ex_neg(ex_t *r, const ex_t *x);
int ex_add(ex_t *r, const ex_t *x, const ex_t *y);
int ex_sub(ex_t *r, const ex_t *x, const ex_t *y);
int ex_mul(ex_t *r, const ex_t *x, const ex_t *y);
int ex_div(ex_t *r, const ex_t *x, const ex_t *y);
int ex_sqrt(ex_t *r, const ex_t *x);                       /* x rational >= 0 */
int ex_powi(ex_t *r, const ex_t *x, int64_t n);            /* |n| <= 64 */
int ex_iroot(ex_t *r, const ex_t *x, int64_t n);           /* exact n-th root of a rational, n >= 2 */

/*
 * Exact trig of special angles (multiples of 30 and 45 degrees).
 * fn: 0 sin, 1 cos, 2 tan; mode: 0 deg, 1 rad, 2 grad.
 * Returns 0 and fills *out, -1 if there is no exact form, -2 if undefined (tan 90).
 */
int ex_trig(int fn, const ex_t *arg, int mode, ex_t *out);
/* Exact inverse trig of special values. fn: 0 asin, 1 acos, 2 atan. */
int ex_itrig(int fn, const ex_t *arg, int mode, ex_t *out);
/* Convert a rational number of degrees to the given angle mode. */
int ex_deg_to_mode(const ex_t *deg, int mode, ex_t *out);

/* Full-precision value (45+ digits) and its single correct rounding to 16 digits. */
void ex_to_xn(xnum_t *r, const ex_t *x);
dec_status ex_to_dec(dec_t *r, const ex_t *x);

/*
 * Text form split for display: sign, numerator text, denominator text ("" if 1).
 * ascii = 1 uses "sqrt(5)" / "pi"; ascii = 0 uses UTF-8 "√5" / "π".
 */
typedef struct ex_text {
    char neg;
    char num[48];
    char den[24];
} ex_text_t;
void ex_format(const ex_t *x, int ascii, ex_text_t *out);

/* Squarefree split of n: n = k*k*s. Returns -1 if n > EX_SQRT_LIMIT. */
int ex_sqrt_split(uint64_t n, uint64_t *k, uint64_t *s);

#endif /* TELECO_VALUES_EXACT_H */
