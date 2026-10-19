/*
 * Scientific functions on dec_t (16 digits, correctly rounded from a 27+ digit
 * internal computation). Every function sets *r = 0 on a non-OK status.
 * DEC_DOMAIN means "Math ERROR" (undefined input); DEC_OVERFLOW likewise.
 */
#ifndef TELECO_FUNCTIONS_MATHFN_H
#define TELECO_FUNCTIONS_MATHFN_H

#include <stdint.h>
#include "numeric/decimal.h"
#include "numeric/xnum.h"

typedef enum ang { ANG_DEG = 0, ANG_RAD = 1, ANG_GRAD = 2 } ang_t;

/* ---- integers ---- */
/* Returns 0 and sets *out if x is an integer with |x| < 2^63, else -1. */
int dec_to_i64(const dec_t *x, int64_t *out);
dec_status fn_trunc(dec_t *r, const dec_t *x);   /* Int(): toward zero   */
dec_status fn_floor(dec_t *r, const dec_t *x);   /* Intg(): toward -inf  */
dec_status fn_ceil(dec_t *r, const dec_t *x);
dec_status fn_frac(dec_t *r, const dec_t *x);    /* x - Int(x)           */

/* ---- exponentials, logarithms, powers ---- */
dec_status fn_exp(dec_t *r, const dec_t *x);
dec_status fn_exp10(dec_t *r, const dec_t *x);
dec_status fn_ln(dec_t *r, const dec_t *x);
dec_status fn_log10(dec_t *r, const dec_t *x);
dec_status fn_log(dec_t *r, const dec_t *base, const dec_t *x);   /* log_base(x) */
dec_status fn_pow(dec_t *r, const dec_t *x, const dec_t *y);      /* x^y */
dec_status fn_cbrt(dec_t *r, const dec_t *x);
dec_status fn_xroot(dec_t *r, const dec_t *n, const dec_t *x);    /* n-th root of x */

/* ---- trigonometry (angle unit = a) ---- */
dec_status fn_sin(dec_t *r, const dec_t *x, ang_t a);
dec_status fn_cos(dec_t *r, const dec_t *x, ang_t a);
dec_status fn_tan(dec_t *r, const dec_t *x, ang_t a);
dec_status fn_asin(dec_t *r, const dec_t *x, ang_t a);
dec_status fn_acos(dec_t *r, const dec_t *x, ang_t a);
dec_status fn_atan(dec_t *r, const dec_t *x, ang_t a);
dec_status fn_atan2(dec_t *r, const dec_t *y, const dec_t *x, ang_t a); /* angle of (x, y) */

/* ---- hyperbolic ---- */
dec_status fn_sinh(dec_t *r, const dec_t *x);
dec_status fn_cosh(dec_t *r, const dec_t *x);
dec_status fn_tanh(dec_t *r, const dec_t *x);
dec_status fn_asinh(dec_t *r, const dec_t *x);
dec_status fn_acosh(dec_t *r, const dec_t *x);
dec_status fn_atanh(dec_t *r, const dec_t *x);

/*
 * Same functions taking a full-precision xnum argument (used when the argument is
 * an exact value such as pi/5, so it is not rounded to 16 digits first).
 */
dec_status fx_exp(dec_t *r, const xnum_t *x);
dec_status fx_exp10(dec_t *r, const xnum_t *x);
dec_status fx_ln(dec_t *r, const xnum_t *x);
dec_status fx_log10(dec_t *r, const xnum_t *x);
dec_status fx_log(dec_t *r, const xnum_t *base, const xnum_t *x);
dec_status fx_pow(dec_t *r, const xnum_t *x, const xnum_t *y);
dec_status fx_cbrt(dec_t *r, const xnum_t *x);
dec_status fx_xroot(dec_t *r, const xnum_t *n, const xnum_t *x);
dec_status fx_sin(dec_t *r, const xnum_t *x, ang_t a);
dec_status fx_cos(dec_t *r, const xnum_t *x, ang_t a);
dec_status fx_tan(dec_t *r, const xnum_t *x, ang_t a);
dec_status fx_asin(dec_t *r, const xnum_t *x, ang_t a);
dec_status fx_acos(dec_t *r, const xnum_t *x, ang_t a);
dec_status fx_atan(dec_t *r, const xnum_t *x, ang_t a);
dec_status fx_atan2(dec_t *r, const xnum_t *y, const xnum_t *x, ang_t a);
dec_status fx_sinh(dec_t *r, const xnum_t *x);
dec_status fx_cosh(dec_t *r, const xnum_t *x);
dec_status fx_tanh(dec_t *r, const xnum_t *x);
dec_status fx_asinh(dec_t *r, const xnum_t *x);
dec_status fx_acosh(dec_t *r, const xnum_t *x);
dec_status fx_atanh(dec_t *r, const xnum_t *x);

/* ---- combinatorics ---- */
dec_status fn_fact(dec_t *r, const dec_t *x);                      /* 0 <= x <= 69 integer */
dec_status fn_npr(dec_t *r, const dec_t *n, const dec_t *k);
dec_status fn_ncr(dec_t *r, const dec_t *n, const dec_t *k);

/* ---- constants ---- */
void fn_pi(dec_t *r);
void fn_e(dec_t *r);

/* ---- angle conversion ---- */
dec_status fn_convert_angle(dec_t *r, const dec_t *x, ang_t from, ang_t to);

#endif /* TELECO_FUNCTIONS_MATHFN_H */
