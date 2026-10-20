/*
 * Result formatting: Norm 1/2, Fix, Sci, Eng, exact fractions / surds / pi.
 * fmt_value fills a structured result (for the LCD renderer) and fmt_text
 * flattens it for the command line and tests.
 */
#ifndef TELECO_CALC_FORMAT_H
#define TELECO_CALC_FORMAT_H

#include <stddef.h>
#include "state/ctx.h"

enum { FR_DEC = 0, FR_FRAC, FR_MIXED, FR_EXACT, FR_CPLX, FR_MAT, FR_VCT };

typedef struct fmt_part {
    uint8_t kind;        /* FR_DEC / FR_FRAC / FR_MIXED / FR_EXACT */
    char neg;
    char whole[16];      /* mixed fraction integer part                          */
    char num[48];        /* decimal digits, or numerator text (may hold sqrt/pi) */
    char den[24];        /* denominator text, "" when none                       */
    char exp[8];         /* "-5" for the x10^exp part, "" when none              */
} fmt_part_t;

typedef struct fmt_res {
    uint8_t kind;
    fmt_part_t p;        /* real part / the number            */
    fmt_part_t q;        /* imaginary part for FR_CPLX        */
    uint8_t rows, cols;  /* matrices                          */
    uint8_t re_zero;     /* complex: real part is zero        */
    uint8_t im_one;      /* complex: |imaginary part| is 1    */
    uint8_t polar;       /* complex shown as r<theta          */
} fmt_res_t;

/*
 * exact != 0: show fractions / surds / pi when an exact form exists (and is short
 * enough); otherwise always decimal.
 */
void fmt_value(const ctx_t *c, const val_t *v, int exact, fmt_res_t *out);
/* A real number with the current display mode. */
void fmt_real(const ctx_t *c, const num_t *n, int exact, fmt_res_t *out);
/* Plain text. ascii != 0 avoids UTF-8 (sqrt(, pi). Returns length. */
size_t fmt_text(const fmt_res_t *r, int ascii, char *buf, size_t cap);
/* Convenience: format ctx-mode text of a value. */
size_t fmt_value_text(const ctx_t *c, const val_t *v, int exact, int ascii, char *buf, size_t cap);

/* Round a decimal to 'sig' significant digits (half away from zero). */
dec_status dec_round_sig(dec_t *r, const dec_t *x, int sig);
/* Round according to the display mode (the Rnd( function). */
dec_status dec_round_display(const ctx_t *c, dec_t *r, const dec_t *x);

#endif
