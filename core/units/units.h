/* Physical constants (CODATA 2022 via tools/gen_constants.py) and unit conversions. */
#ifndef TELECO_UNITS_UNITS_H
#define TELECO_UNITS_UNITS_H

#include "numeric/decimal.h"

typedef struct const_info {
    const char *name, *symbol, *value, *unit;
} const_info_t;

int  const_count(void);
const const_info_t *const_get(int index);                 /* 0-based, NULL when out of range */
dec_status const_value(int index, dec_t *out);

typedef struct conv_info {
    const char *from, *to;
} conv_info_t;

int  conv_count(void);
const conv_info_t *conv_get(int index);                   /* 0-based */
dec_status conv_apply(int index, const dec_t *x, dec_t *out);

#endif
