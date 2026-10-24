/*
 * Calculator context: everything that persists between calculations.
 * All storage is inline (no heap).
 */
#ifndef TELECO_STATE_CTX_H
#define TELECO_STATE_CTX_H

#include "values/val.h"
#include "values/matrix.h"
#include "parser/token.h"
#include "calc/errors.h"

enum { FMT_NORM1 = 0, FMT_NORM2, FMT_FIX, FMT_SCI, FMT_ENG };

#define N_MATS   9      /* MatA..MatD, MatAns, 4 temporaries   */
#define MAT_NAMED 5
#define N_VCTS   9
#define MAT_ANS  4

typedef struct mat { uint8_t r, c; num_t e[MAT_MAX * MAT_MAX]; } mat_t;
typedef struct vct { uint8_t r; num_t e[VEC_MAX]; } vct_t;

typedef struct ctx {
    ang_t    angle;            /* Deg / Rad / Grad                          */
    uint8_t  cplx;             /* complex mode on                           */
    uint8_t  fmt;              /* FMT_*                                     */
    uint8_t  fmt_digits;       /* digits for Fix / Sci (0..9)               */
    uint8_t  frac_improper;    /* d/c display instead of mixed fractions    */
    uint8_t  polar;            /* complex results as r<theta                 */
    uint8_t  base;             /* 0 = decimal COMP; 2, 8, 10, 16 base-n     */
    val_t    var[N_VARS];      /* A B C D E F X Y M                         */
    val_t    ans, preans;
    mat_t    mats[N_MATS];     /* r == 0: undefined                          */
    vct_t    vcts[N_VCTS];
    uint16_t tmp_used;         /* bit i: temporary slot MAT_NAMED+i in use (matrices) */
    uint16_t tmp_used_v;
    uint32_t rng;              /* xorshift state                            */
    uint32_t budget;           /* remaining VM steps for this evaluation    */
    num_t    aux;              /* second result of Pol( / Rec( (theta / y)  */
    uint8_t  has_aux;
} ctx_t;

void ctx_init(ctx_t *c);

#endif
