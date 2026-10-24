/*
 * Statistics: one-variable summary, two-variable regression (linear, quadratic,
 * logarithmic, exponential, a*b^x, power, inverse) with integer frequencies.
 * Sums use num_t, so integer data give exact fractions and surds.
 */
#ifndef TELECO_STATS_STAT_H
#define TELECO_STATS_STAT_H

#include "values/num.h"
#include "calc/errors.h"

#define STAT_ROWS 80

enum stat_type {
    ST_1VAR = 0, ST_LIN,      /* y = a + b x          */
    ST_QUAD,                  /* y = a + b x + c x^2  */
    ST_LOG,                   /* y = a + b ln x       */
    ST_EXP,                   /* y = a e^(b x)        */
    ST_AB,                    /* y = a b^x            */
    ST_POW,                   /* y = a x^b            */
    ST_INV                    /* y = a + b / x        */
};

enum stat_var {
    SV_N = 0, SV_SUMX, SV_SUMX2, SV_XBAR, SV_SIGX, SV_SX,
    SV_SUMY, SV_SUMY2, SV_YBAR, SV_SIGY, SV_SY, SV_SUMXY,
    SV_A, SV_B, SV_C, SV_R,
    SV_MINX, SV_MAXX, SV_MINY, SV_MAXY, SV_Q1, SV_MED, SV_Q3,
    SV_SUMX3, SV_SUMX4, SV_SUMX2Y,
    SV_COUNT
};

typedef struct stat {
    uint8_t type;
    uint8_t rows;
    dec_t   x[STAT_ROWS];
    dec_t   y[STAT_ROWS];
    uint8_t f[STAT_ROWS];
} stat_t;

void stat_init(stat_t *s, int type);
calc_err_t stat_add(stat_t *s, const dec_t *x, const dec_t *y, int freq);
calc_err_t stat_delete(stat_t *s, int row);                 /* 0-based row */
calc_err_t stat_var(const stat_t *s, int id, ang_t unused, num_t *out);
calc_err_t stat_yhat(const stat_t *s, const num_t *x, num_t *out);
calc_err_t stat_xhat(const stat_t *s, const num_t *y, num_t *out);

#endif
