/* Internal to core/numeric: not part of the public API. */
#ifndef TELECO_NUMERIC_DECIMAL_INTERNAL_H
#define TELECO_NUMERIC_DECIMAL_INTERNAL_H

#include "numeric/decimal.h"
#include "numeric/wide.h"

/*
 * Build a dec_t from the exact-plus-sticky value  (n + f) * 10^e,  where
 * f is a fraction in (0, 1) when `sticky` is non-zero and 0 otherwise.
 *
 * Normalises to 16 digits, rounds half-to-even, then range-checks. On overflow
 * or underflow *r is set to zero and the matching status is returned.
 */
dec_status dec_pack_internal(dec_t *r, int neg, u128 n, int sticky, int32_t e);

#endif /* TELECO_NUMERIC_DECIMAL_INTERNAL_H */
