/*
 * Teleco fixed-size decimal number: 16 significant digits, exponent -99..99.
 * See docs/numeric-spec.md for the full specification.
 */
#ifndef TELECO_NUMERIC_DECIMAL_H
#define TELECO_NUMERIC_DECIMAL_H

#include <stddef.h>
#include <stdint.h>

#define DEC_DIGITS      16
#define DEC_EXP_MAX     99
#define DEC_EXP_MIN     (-99)
#define DEC_MANT_MIN    1000000000000000ULL   /* 10^15 */
#define DEC_MANT_LIMIT  10000000000000000ULL  /* 10^16 (exclusive upper bound) */

/* Buffer size that is always large enough for dec_to_str(). */
#define DEC_STR_MAX     32

/*
 * value = (-1)^neg * m * 10^(exp - 15)
 *
 * Canonical form: either m == 0 (then exp == 0 and neg == 0), or
 * DEC_MANT_MIN <= m < DEC_MANT_LIMIT. Compare values with dec_cmp().
 */
typedef struct dec {
    uint64_t m;
    int16_t  exp;
    uint8_t  neg;
} dec_t;

typedef enum dec_status {
    DEC_OK = 0,
    DEC_UNDERFLOW,     /* non-fatal: result flushed to zero */
    DEC_OVERFLOW,      /* fatal */
    DEC_DIV_BY_ZERO,   /* fatal */
    DEC_DOMAIN,        /* fatal: 0/0, sqrt(negative), ... */
    DEC_PARSE          /* fatal: malformed input string */
} dec_status;

/* Non-zero for statuses a calculator should report as an error. */
#define DEC_STATUS_IS_ERROR(s) ((s) >= DEC_OVERFLOW)

const char *dec_status_name(dec_status s);

/* ---- construction and inspection ---------------------------------------- */

void dec_zero(dec_t *r);
int  dec_is_zero(const dec_t *a);
int  dec_is_valid(const dec_t *a);               /* canonical-form check */
int  dec_cmp(const dec_t *a, const dec_t *b);    /* -1, 0, 1 */
void dec_neg(dec_t *r, const dec_t *a);
void dec_abs(dec_t *r, const dec_t *a);

dec_status dec_from_i64(dec_t *r, int64_t v);

/*
 * Parse "[+-]digits[.digits][(e|E)[+-]digits]". Rounds correctly if more than
 * 16 digits are given. On any non-OK status *r is set to zero.
 */
dec_status dec_from_str(dec_t *r, const char *s);

/*
 * Write the canonical test format "-d.dddddddddddddddE+XX" (or "0").
 * Returns the length written (excluding the terminating NUL), or 0 if cap is
 * smaller than DEC_STR_MAX.
 */
size_t dec_to_str(const dec_t *a, char *buf, size_t cap);

/* ---- arithmetic (correctly rounded; r may alias a and/or b) -------------- */

dec_status dec_add(dec_t *r, const dec_t *a, const dec_t *b);
dec_status dec_sub(dec_t *r, const dec_t *a, const dec_t *b);
dec_status dec_mul(dec_t *r, const dec_t *a, const dec_t *b);
dec_status dec_div(dec_t *r, const dec_t *a, const dec_t *b);
dec_status dec_sqrt(dec_t *r, const dec_t *a);

#endif /* TELECO_NUMERIC_DECIMAL_H */
