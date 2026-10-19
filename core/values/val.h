/*
 * val_t: what lives on the evaluation stack and in variables.
 *   V_REAL : re
 *   V_CPLX : re + im*i
 *   V_MAT / V_VCT : handle into the context's matrix pool (mat)
 */
#ifndef TELECO_VALUES_VAL_H
#define TELECO_VALUES_VAL_H

#include "values/num.h"

enum { V_REAL = 0, V_CPLX = 1, V_MAT = 2, V_VCT = 3 };

typedef struct val {
    uint8_t k;
    uint8_t mat;
    num_t re;
    num_t im;
} val_t;

void val_real(val_t *v, const num_t *n);
void val_int(val_t *v, int64_t n);
void val_zero(val_t *v);

#endif
