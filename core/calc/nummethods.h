/*
 * Numerical methods on 16-digit decimals: derivative (Ridders), integral
 * (adaptive Gauss-Kronrod 7/15) and root finding (bracket search + Brent).
 * The function being analysed is supplied as a callback.
 */
#ifndef TELECO_CALC_NUMMETHODS_H
#define TELECO_CALC_NUMMETHODS_H

#include "numeric/decimal.h"
#include "calc/errors.h"

/* y = f(x). Return CE_OK, or an error (CE_MATH means "f undefined at x"). */
typedef calc_err_t (*nm_fn)(void *user, const dec_t *x, dec_t *y);

calc_err_t nm_derivative(nm_fn f, void *user, const dec_t *x0, dec_t *out);
calc_err_t nm_integrate(nm_fn f, void *user, const dec_t *a, const dec_t *b, dec_t *out);
/* Root of f(x) = 0 nearest the guess. *resid receives f(root). */
calc_err_t nm_solve(nm_fn f, void *user, const dec_t *guess, dec_t *root, dec_t *resid);

#endif
