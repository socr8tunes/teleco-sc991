/* Standard normal distribution: P(t) left tail, Q(t) = area from 0 to t, R(t) right tail. */
#ifndef TELECO_FUNCTIONS_DIST_H
#define TELECO_FUNCTIONS_DIST_H

#include "numeric/decimal.h"

enum { NORM_P = 0, NORM_Q = 1, NORM_R = 2 };

dec_status dist_norm(int which, dec_t *r, const dec_t *t);

#endif
