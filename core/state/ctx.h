/*
 * Calculator context: everything that persists between calculations.
 * All storage is inline (no heap).
 */
#ifndef TELECO_STATE_CTX_H
#define TELECO_STATE_CTX_H

#include "values/val.h"
#include "values/matrix.h"
#include "stats/stat.h"
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
    stat_t   stat;             /* STAT editor data and type                  */
    uint16_t tmp_used;         /* bit i: temporary slot MAT_NAMED+i in use (matrices) */
    uint16_t tmp_used_v;
    uint32_t rng;              /* xorshift state                            */
    uint32_t budget;           /* remaining VM steps for this evaluation    */
    val_t    res[4];           /* multi-valued results (roots x1..x3, simultaneous solutions) */
    uint8_t  nres;
    uint8_t  res_kind;         /* 1 = roots (x1, x2, x3), 2 = unknowns (x, y, z)                */
    num_t    aux;              /* second result of Pol( / Rec( (theta / y)  */
    uint8_t  has_aux;          /* 1 = SOLVE residual (L-R), 2 = second Pol/Rec value */
} ctx_t;

void ctx_init(ctx_t *c);

/*
 * Temporarily override display settings for one formatting call. The context is
 * ~14 KB, so copying it just to change a field would blow the MCU stack; the
 * override is applied in place and undone by ctx_fmt_restore (not re-entrant).
 * fmt < 0 / improper < 0 leave that setting alone.
 */
typedef struct ctx_fmt_saved { uint8_t fmt, frac_improper; } ctx_fmt_saved_t;
static inline ctx_fmt_saved_t ctx_fmt_override(const ctx_t *c, int fmt, int improper)
{
    ctx_t *m = (ctx_t *)c;
    ctx_fmt_saved_t s;
    s.fmt = m->fmt;
    s.frac_improper = m->frac_improper;
    if (fmt >= 0) m->fmt = (uint8_t)fmt;
    if (improper >= 0) m->frac_improper = (uint8_t)improper;
    return s;
}
static inline void ctx_fmt_restore(const ctx_t *c, ctx_fmt_saved_t s)
{
    ctx_t *m = (ctx_t *)c;
    m->fmt = s.fmt;
    m->frac_improper = s.frac_improper;
}

#endif
