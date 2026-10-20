/*
 * Complex numbers on top of num_t (exact parts stay exact: (1+i)(1-i) = 2,
 * sqrt(-4) = 2i, |3+4i| = 5, ...). Complex trig uses radians.
 */
#ifndef TELECO_VALUES_CPLX_H
#define TELECO_VALUES_CPLX_H

#include "values/num.h"

typedef struct cx {
    num_t re, im;
} cx_t;

void cx_make(cx_t *r, const num_t *re, const num_t *im);
void cx_from_real(cx_t *r, const num_t *re);
int  cx_is_real(const cx_t *a);

dec_status cx_neg(cx_t *r, const cx_t *a);
dec_status cx_conj(cx_t *r, const cx_t *a);
dec_status cx_add(cx_t *r, const cx_t *a, const cx_t *b);
dec_status cx_sub(cx_t *r, const cx_t *a, const cx_t *b);
dec_status cx_mul(cx_t *r, const cx_t *a, const cx_t *b);
dec_status cx_div(cx_t *r, const cx_t *a, const cx_t *b);
dec_status cx_pow(cx_t *r, const cx_t *a, const cx_t *b);
dec_status cx_sqrt(cx_t *r, const cx_t *a);
dec_status cx_abs(num_t *r, const cx_t *a);
dec_status cx_arg(num_t *r, const cx_t *a, ang_t mode);
dec_status cx_exp(cx_t *r, const cx_t *a);
dec_status cx_ln(cx_t *r, const cx_t *a);
dec_status cx_log10(cx_t *r, const cx_t *a);
dec_status cx_sin(cx_t *r, const cx_t *a);
dec_status cx_cos(cx_t *r, const cx_t *a);
dec_status cx_tan(cx_t *r, const cx_t *a);
dec_status cx_sinh(cx_t *r, const cx_t *a);
dec_status cx_cosh(cx_t *r, const cx_t *a);
dec_status cx_tanh(cx_t *r, const cx_t *a);
/* r(cos t + i sin t) with t in angle units `mode`. */
dec_status cx_polar(cx_t *r, const num_t *mag, const num_t *theta, ang_t mode);

#endif
