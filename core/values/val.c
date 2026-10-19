#include <string.h>
#include "values/val.h"

void val_real(val_t *v, const num_t *n)
{
    memset(v, 0, sizeof *v);
    v->k = V_REAL;
    v->re = *n;
    num_zero(&v->im);
}

void val_int(val_t *v, int64_t n)
{
    num_t t;
    num_from_int(&t, n);
    val_real(v, &t);
}

void val_zero(val_t *v) { val_int(v, 0); }
