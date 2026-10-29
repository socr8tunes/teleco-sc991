/* Polynomial roots (degree 2 and 3) and linear systems, exact where the inputs are exact. */
#ifndef TELECO_SOLVERS_EQN_H
#define TELECO_SOLVERS_EQN_H

#include "values/cplx.h"
#include "calc/errors.h"

/* a x^2 + b x + c = 0 : two roots (complex when the discriminant is negative). */
calc_err_t eqn_quad(const num_t c[3], cx_t roots[2]);
/* a x^3 + b x^2 + c x + d = 0 : three roots, real roots first in ascending order. */
calc_err_t eqn_cubic(const num_t c[4], cx_t roots[3]);
/* n x n linear system, a is n rows of (n coefficients + constant). Solution x[n]. */
calc_err_t eqn_simul(int n, const num_t *a, num_t *x);

#endif
