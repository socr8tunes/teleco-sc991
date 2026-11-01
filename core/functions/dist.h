/* Standard normal distribution: P(t) left tail, Q(t) = area from 0 to t, R(t) right tail. */
#ifndef TELECO_FUNCTIONS_DIST_H
#define TELECO_FUNCTIONS_DIST_H

#include <stdint.h>
#include "numeric/decimal.h"

enum { NORM_P = 0, NORM_Q = 1, NORM_R = 2 };

dec_status dist_norm(int which, dec_t *r, const dec_t *t);

/*
 * Discrete distributions (extended precision internally, no underflow).
 *   dist_binom_pd(r, x, n, p)   P(X = x)    X ~ B(n, p), 0 <= x <= n integers, n <= DIST_N_MAX
 *   dist_binom_cd(r, x, n, p)   P(X <= x)
 *   dist_poisson_pd(r, x, lam)  P(X = x)    X ~ Poisson(lam), lam <= DIST_LAMBDA_MAX
 *   dist_poisson_cd(r, x, lam)  P(X <= x)
 *   dist_inv_norm(r, area)      t with P(t) = area, 0 < area < 1 (standard normal)
 * DEC_DOMAIN on invalid arguments.
 */
#define DIST_N_MAX      10000
#define DIST_LAMBDA_MAX 10000

dec_status dist_binom_pd(dec_t *r, int64_t x, int64_t n, const dec_t *p);
dec_status dist_binom_cd(dec_t *r, int64_t x, int64_t n, const dec_t *p);
dec_status dist_poisson_pd(dec_t *r, int64_t x, const dec_t *lam);
dec_status dist_poisson_cd(dec_t *r, int64_t x, const dec_t *lam);
dec_status dist_inv_norm(dec_t *r, const dec_t *area);

#endif
