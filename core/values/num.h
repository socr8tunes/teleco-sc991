/*
 * num_t: a real number as the calculator sees it. The decimal value is always
 * valid; `ex` additionally carries an exact form (fraction, surd, multiple of pi)
 * while one is known. When ex is valid, v == ex_to_dec(ex).
 */
#ifndef TELECO_VALUES_NUM_H
#define TELECO_VALUES_NUM_H

#include "numeric/decimal.h"
#include "values/exact.h"
#include "functions/mathfn.h"

typedef struct num {
    dec_t v;
    ex_t  ex;
} num_t;

void num_zero(num_t *r);
void num_from_int(num_t *r, int64_t n);                 /* exact */
void num_from_dec(num_t *r, const dec_t *d);            /* inexact (typed decimals, function results) */
dec_status num_from_ex(num_t *r, const ex_t *x);        /* exact, v derived */
void num_pi(num_t *r);                                  /* exact pi */
int  num_is_zero(const num_t *a);
int  num_is_exact(const num_t *a);
int  num_cmp(const num_t *a, const num_t *b);           /* by decimal value */
void num_xn(xnum_t *x, const num_t *a);                 /* full-precision value (exact form if known) */

dec_status num_neg(num_t *r, const num_t *a);
dec_status num_abs(num_t *r, const num_t *a);
dec_status num_add(num_t *r, const num_t *a, const num_t *b);
dec_status num_sub(num_t *r, const num_t *a, const num_t *b);
dec_status num_mul(num_t *r, const num_t *a, const num_t *b);
dec_status num_div(num_t *r, const num_t *a, const num_t *b);
dec_status num_sqrt(num_t *r, const num_t *a);
dec_status num_pow(num_t *r, const num_t *a, const num_t *b);        /* a^b */
dec_status num_root(num_t *r, const num_t *n, const num_t *a);       /* n-th root of a */

dec_status num_sin(num_t *r, const num_t *a, ang_t m);
dec_status num_cos(num_t *r, const num_t *a, ang_t m);
dec_status num_tan(num_t *r, const num_t *a, ang_t m);
dec_status num_asin(num_t *r, const num_t *a, ang_t m);
dec_status num_acos(num_t *r, const num_t *a, ang_t m);
dec_status num_atan(num_t *r, const num_t *a, ang_t m);

/* Integer-valued results that are exact when they fit in int64. */
dec_status num_fact(num_t *r, const num_t *a);
dec_status num_npr(num_t *r, const num_t *n, const num_t *k);
dec_status num_ncr(num_t *r, const num_t *n, const num_t *k);

/* Wrap a decimal-only function result. */
dec_status num_wrap(num_t *r, const dec_t *d, dec_status st);

/* Convert angle value (exact aware) between modes: x[from] -> x[to]. */
dec_status num_convert_angle(num_t *r, const num_t *a, ang_t from, ang_t to);

#endif /* TELECO_VALUES_NUM_H */
