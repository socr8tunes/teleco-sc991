/*
 * Elementary functions on xnum (27..36 digit working precision).
 * Used by mathfn.c, which rounds the results to 16 digits.
 *
 * All functions return DEC_OK, DEC_DOMAIN or DEC_OVERFLOW (never touch *r on DOMAIN).
 */
#ifndef TELECO_FUNCTIONS_XFUNC_H
#define TELECO_FUNCTIONS_XFUNC_H

#include "numeric/decimal.h"
#include "numeric/xnum.h"

/* Angle units for trig functions */
#define XF_UNIT_RAD  0
#define XF_UNIT_DEG  1
#define XF_UNIT_GRAD 2

#define XF_TRIG_LIMIT_EXP10 10   /* |x| must be < 10^10 */

dec_status xf_exp(xnum_t *r, const xnum_t *x);
dec_status xf_ln(xnum_t *r, const xnum_t *x);        /* x > 0 */
dec_status xf_log1p(xnum_t *r, const xnum_t *x);     /* x > -1 */
dec_status xf_pow_pos(xnum_t *r, const xnum_t *x, const xnum_t *y); /* x > 0 */

/*
 * sin and cos of x given in `unit`. For DEG/GRAD the reduction is exact; *rzero
 * is set when x is an exact multiple of a quarter turn (then sin/cos are exact
 * 0/+-1 and tan is undefined for odd quadrants). *quad_odd is the quadrant parity.
 */
dec_status xf_sincos(const xnum_t *x, int unit, xnum_t *s, xnum_t *c, int *rzero, int *quad_odd);

dec_status xf_atan(xnum_t *r, const xnum_t *x);
dec_status xf_atan2(xnum_t *r, const xnum_t *y, const xnum_t *x);
dec_status xf_asin(xnum_t *r, const xnum_t *x);
dec_status xf_acos(xnum_t *r, const xnum_t *x);

dec_status xf_sinh(xnum_t *r, const xnum_t *x);
dec_status xf_cosh(xnum_t *r, const xnum_t *x);
dec_status xf_tanh(xnum_t *r, const xnum_t *x);
dec_status xf_asinh(xnum_t *r, const xnum_t *x);
dec_status xf_acosh(xnum_t *r, const xnum_t *x);
dec_status xf_atanh(xnum_t *r, const xnum_t *x);

/* Convert a radian result to the given unit (identity for RAD). */
void xf_rad_to_unit(xnum_t *r, const xnum_t *rad, int unit);

#endif /* TELECO_FUNCTIONS_XFUNC_H */
