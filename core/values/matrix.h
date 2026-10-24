/*
 * Small matrices (up to 4x4) and vectors (up to 3 elements) on num_t, so exact
 * fractions survive: inverse of [[1,2],[3,4]] is [[-2,1],[3/2,-1/2]].
 * A "view" describes storage owned by the caller (the context's pools).
 */
#ifndef TELECO_VALUES_MATRIX_H
#define TELECO_VALUES_MATRIX_H

#include "values/num.h"

#define MAT_MAX 4
#define VEC_MAX 3

typedef struct mv {
    uint8_t r, c;      /* rows, columns (vectors: c == 1)  */
    num_t  *e;         /* row-major, r*c elements          */
} mv_t;

/* All operations write into out (which must have room for the result) and set out->r/c. */
dec_status mx_add(mv_t *out, const mv_t *a, const mv_t *b);          /* DEC_DOMAIN on dimension mismatch */
dec_status mx_sub(mv_t *out, const mv_t *a, const mv_t *b);
dec_status mx_mul(mv_t *out, const mv_t *a, const mv_t *b);          /* a (r x k) * b (k x c) */
dec_status mx_scale(mv_t *out, const mv_t *a, const num_t *k);
dec_status mx_div_scalar(mv_t *out, const mv_t *a, const num_t *k);
dec_status mx_neg(mv_t *out, const mv_t *a);
dec_status mx_trn(mv_t *out, const mv_t *a);                          /* out must not alias a */
dec_status mx_det(num_t *out, const mv_t *a);
dec_status mx_inv(mv_t *out, const mv_t *a);                          /* out must not alias a */
dec_status mx_identity(mv_t *out, int n);
dec_status mx_dot(num_t *out, const mv_t *a, const mv_t *b);
dec_status mx_cross(mv_t *out, const mv_t *a, const mv_t *b);        /* 3-vectors */
dec_status mx_norm(num_t *out, const mv_t *a);
dec_status mx_angle(num_t *out, const mv_t *a, const mv_t *b, ang_t mode);
dec_status mx_unit(mv_t *out, const mv_t *a);

#endif
