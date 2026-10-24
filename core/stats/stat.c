#include <string.h>
#include "stats/stat.h"
#include "values/matrix.h"

#define CHK(x) do { dec_status _s = (x); if (DEC_STATUS_IS_ERROR(_s)) return CE_MATH; } while (0)

void stat_init(stat_t *s, int type)
{
    memset(s, 0, sizeof *s);
    s->type = (uint8_t)type;
}

calc_err_t stat_add(stat_t *s, const dec_t *x, const dec_t *y, int freq)
{
    if (s->rows >= STAT_ROWS) return CE_RANGE;
    if (freq < 1 || freq > 99) return CE_RANGE;
    s->x[s->rows] = *x;
    if (y) s->y[s->rows] = *y; else dec_zero(&s->y[s->rows]);
    s->f[s->rows] = (uint8_t)freq;
    s->rows++;
    return CE_OK;
}

calc_err_t stat_delete(stat_t *s, int row)
{
    int i;
    if (row < 0 || row >= s->rows) return CE_RANGE;
    for (i = row; i + 1 < s->rows; i++) { s->x[i] = s->x[i + 1]; s->y[i] = s->y[i + 1]; s->f[i] = s->f[i + 1]; }
    s->rows--;
    return CE_OK;
}

/* ---- transforms ---------------------------------------------------------- */

static int two_var(const stat_t *s) { return s->type != ST_1VAR; }

static calc_err_t tx(const stat_t *s, const dec_t *x, num_t *out)
{
    dec_t d;
    switch (s->type) {
    case ST_LOG: case ST_POW:
        CHK(fn_ln(&d, x));
        num_from_dec(out, &d);
        return CE_OK;
    case ST_INV: {
        num_t one, v;
        num_from_dec(&v, x);
        num_from_int(&one, 1);
        CHK(num_div(out, &one, &v));
        return CE_OK;
    }
    default: {
        /* data typed as integers stay exact */
        int64_t iv;
        if (dec_to_i64(x, &iv) == 0 && iv > -900000000000000LL && iv < 900000000000000LL) num_from_int(out, iv);
        else num_from_dec(out, x);
        return CE_OK;
    }
    }
}

static calc_err_t ty(const stat_t *s, const dec_t *y, num_t *out)
{
    dec_t d;
    if (s->type == ST_EXP || s->type == ST_AB || s->type == ST_POW) {
        CHK(fn_ln(&d, y));
        num_from_dec(out, &d);
        return CE_OK;
    }
    {
        int64_t iv;
        if (dec_to_i64(y, &iv) == 0 && iv > -900000000000000LL && iv < 900000000000000LL) num_from_int(out, iv);
        else num_from_dec(out, y);
    }
    return CE_OK;
}

typedef struct sums {
    num_t n, sx, sy, sx2, sy2, sxy, sx3, sx4, sx2y;   /* of transformed data */
    num_t xbar, ybar, Sxx, Syy, Sxy;
} sums_t;

static calc_err_t accumulate(const stat_t *s, sums_t *m, int raw)
{
    int i;
    num_t f, x, y, t, u;
    stat_t raw_view;
    const stat_t *v = s;
    if (s->rows == 0) return CE_MATH;
    if (raw) { raw_view = *s; raw_view.type = (s->type == ST_1VAR) ? ST_1VAR : ST_LIN; v = &raw_view; }
    num_from_int(&m->n, 0); num_from_int(&m->sx, 0); num_from_int(&m->sy, 0);
    num_from_int(&m->sx2, 0); num_from_int(&m->sy2, 0); num_from_int(&m->sxy, 0);
    num_from_int(&m->sx3, 0); num_from_int(&m->sx4, 0); num_from_int(&m->sx2y, 0);
    for (i = 0; i < v->rows; i++) {
        calc_err_t e = tx(v, &v->x[i], &x);
        if (e) return e;
        if (two_var(v)) { e = ty(v, &v->y[i], &y); if (e) return e; } else num_from_int(&y, 0);
        num_from_int(&f, v->f[i]);
        CHK(num_add(&m->n, &m->n, &f));
        CHK(num_mul(&t, &f, &x));  CHK(num_add(&m->sx, &m->sx, &t));
        CHK(num_mul(&t, &t, &x));  CHK(num_add(&m->sx2, &m->sx2, &t));
        CHK(num_mul(&u, &t, &y));  CHK(num_add(&m->sx2y, &m->sx2y, &u));
        CHK(num_mul(&t, &t, &x));  CHK(num_add(&m->sx3, &m->sx3, &t));
        CHK(num_mul(&t, &t, &x));  CHK(num_add(&m->sx4, &m->sx4, &t));
        CHK(num_mul(&t, &f, &y));  CHK(num_add(&m->sy, &m->sy, &t));
        CHK(num_mul(&u, &t, &y));  CHK(num_add(&m->sy2, &m->sy2, &u));
        CHK(num_mul(&u, &t, &x));  CHK(num_add(&m->sxy, &m->sxy, &u));
    }
    CHK(num_div(&m->xbar, &m->sx, &m->n));
    CHK(num_div(&m->ybar, &m->sy, &m->n));
    /* second pass for the centred sums (accurate for offset data) */
    num_from_int(&m->Sxx, 0); num_from_int(&m->Syy, 0); num_from_int(&m->Sxy, 0);
    for (i = 0; i < v->rows; i++) {
        num_t dx, dy;
        calc_err_t e = tx(v, &v->x[i], &x);
        if (e) return e;
        if (two_var(v)) { e = ty(v, &v->y[i], &y); if (e) return e; } else num_from_int(&y, 0);
        num_from_int(&f, v->f[i]);
        CHK(num_sub(&dx, &x, &m->xbar));
        CHK(num_sub(&dy, &y, &m->ybar));
        CHK(num_mul(&t, &dx, &dx)); CHK(num_mul(&t, &t, &f)); CHK(num_add(&m->Sxx, &m->Sxx, &t));
        CHK(num_mul(&t, &dy, &dy)); CHK(num_mul(&t, &t, &f)); CHK(num_add(&m->Syy, &m->Syy, &t));
        CHK(num_mul(&t, &dx, &dy)); CHK(num_mul(&t, &t, &f)); CHK(num_add(&m->Sxy, &m->Sxy, &t));
    }
    return CE_OK;
}

/* ---- order statistics ---------------------------------------------------- */

typedef struct pt { dec_t v; int f; } pt_t;

static int sorted(const stat_t *s, int use_y, pt_t *p)
{
    int i, j, n = s->rows;
    for (i = 0; i < n; i++) { p[i].v = use_y ? s->y[i] : s->x[i]; p[i].f = s->f[i]; }
    for (i = 1; i < n; i++) {
        pt_t k = p[i];
        for (j = i - 1; j >= 0 && dec_cmp(&p[j].v, &k.v) > 0; j--) p[j + 1] = p[j];
        p[j + 1] = k;
    }
    return n;
}

/* k-th (0-based) element of the expanded sorted list */
static dec_t kth(const pt_t *p, int n, int k)
{
    int i, c = 0;
    for (i = 0; i < n; i++) { c += p[i].f; if (k < c) return p[i].v; }
    return p[n - 1].v;
}

/* median of expanded elements [lo, hi) */
static calc_err_t median_range(const pt_t *p, int n, int lo, int hi, num_t *out)
{
    int len = hi - lo;
    num_t a, b, two;
    dec_t d1, d2;
    if (len <= 0) return CE_MATH;
    d1 = kth(p, n, lo + (len - 1) / 2);
    d2 = kth(p, n, lo + len / 2);
    num_from_dec(&a, &d1); num_from_dec(&b, &d2);
    {
        int64_t iv;
        if (dec_to_i64(&d1, &iv) == 0) num_from_int(&a, iv);
        if (dec_to_i64(&d2, &iv) == 0) num_from_int(&b, iv);
    }
    if (len & 1) { *out = a; return CE_OK; }
    num_from_int(&two, 2);
    CHK(num_add(&a, &a, &b));
    CHK(num_div(out, &a, &two));
    return CE_OK;
}

static int total_n(const stat_t *s) { int i, t = 0; for (i = 0; i < s->rows; i++) t += s->f[i]; return t; }

static calc_err_t order_stat(const stat_t *s, int id, num_t *out)
{
    pt_t p[STAT_ROWS];
    int use_y = (id == SV_MINY || id == SV_MAXY), n, tn;
    dec_t d;
    int64_t iv;
    if (s->rows == 0) return CE_MATH;
    n = sorted(s, use_y, p);
    tn = total_n(s);
    switch (id) {
    case SV_MINX: case SV_MINY: d = p[0].v; break;
    case SV_MAXX: case SV_MAXY: d = p[n - 1].v; break;
    case SV_MED: return median_range(p, n, 0, tn, out);
    case SV_Q1: return median_range(p, n, 0, tn / 2, out);
    case SV_Q3: return median_range(p, n, (tn + 1) / 2, tn, out);
    default: return CE_ARG;
    }
    if (dec_to_i64(&d, &iv) == 0) num_from_int(out, iv); else num_from_dec(out, &d);
    return CE_OK;
}

/* ---- regression ----------------------------------------------------------- */

typedef struct reg { num_t a, b, c, r; } reg_t;

static calc_err_t regress(const stat_t *s, reg_t *g)
{
    sums_t m;
    calc_err_t e;
    num_t t, u;
    dec_t d;
    if (!two_var(s)) return CE_ARG;
    e = accumulate(s, &m, 0);
    if (e) return e;
    num_from_int(&g->c, 0);
    if (s->type == ST_QUAD) {
        /* normal equations for a + b x + c x^2 solved with the exact matrix inverse */
        mv_t A, inv, rhs, sol;
        num_t ae[9], ie[9], re_[3], se[3];
        A.r = A.c = 3; A.e = ae; inv.e = ie; rhs.e = re_; sol.e = se;
        ae[0] = m.n;  ae[1] = m.sx;  ae[2] = m.sx2;
        ae[3] = m.sx; ae[4] = m.sx2; ae[5] = m.sx3;
        ae[6] = m.sx2; ae[7] = m.sx3; ae[8] = m.sx4;
        rhs.r = 3; rhs.c = 1;
        re_[0] = m.sy; re_[1] = m.sxy; re_[2] = m.sx2y;
        CHK(mx_inv(&inv, &A));
        CHK(mx_mul(&sol, &inv, &rhs));
        g->a = se[0]; g->b = se[1]; g->c = se[2];
        num_from_int(&g->r, 0);
        return CE_OK;
    }
    if (num_is_zero(&m.Sxx)) return CE_MATH;
    CHK(num_div(&g->b, &m.Sxy, &m.Sxx));
    CHK(num_mul(&t, &g->b, &m.xbar));
    CHK(num_sub(&g->a, &m.ybar, &t));
    if (num_is_zero(&m.Syy)) {
        num_from_int(&g->r, 1);          /* constant y: Casio gives r = 1 here only if slope zero; keep defined */
        num_from_int(&g->r, 0);
    } else {
        CHK(num_mul(&t, &m.Sxx, &m.Syy));
        CHK(num_sqrt(&u, &t));
        CHK(num_div(&g->r, &m.Sxy, &u));
    }
    /* back-transform for exponential families */
    if (s->type == ST_EXP || s->type == ST_POW) {
        CHK(fn_exp(&d, &g->a.v));
        num_from_dec(&g->a, &d);
    } else if (s->type == ST_AB) {
        CHK(fn_exp(&d, &g->a.v));
        num_from_dec(&g->a, &d);
        CHK(fn_exp(&d, &g->b.v));
        num_from_dec(&g->b, &d);
    }
    return CE_OK;
}

calc_err_t stat_var(const stat_t *s, int id, ang_t unused, num_t *out)
{
    sums_t m;
    calc_err_t e;
    num_t t, nm1, one;
    (void)unused;

    if (id >= SV_MINX && id <= SV_Q3) return order_stat(s, id, out);
    if (id == SV_A || id == SV_B || id == SV_C || id == SV_R) {
        reg_t g;
        e = regress(s, &g);
        if (e) return e;
        *out = id == SV_A ? g.a : id == SV_B ? g.b : id == SV_C ? g.c : g.r;
        return CE_OK;
    }
    e = accumulate(s, &m, 1);
    if (e) return e;
    num_from_int(&one, 1);
    switch (id) {
    case SV_N: *out = m.n; return CE_OK;
    case SV_SUMX: *out = m.sx; return CE_OK;
    case SV_SUMX2: *out = m.sx2; return CE_OK;
    case SV_SUMX3: *out = m.sx3; return CE_OK;
    case SV_SUMX4: *out = m.sx4; return CE_OK;
    case SV_XBAR: *out = m.xbar; return CE_OK;
    case SV_SUMY: *out = m.sy; return CE_OK;
    case SV_SUMY2: *out = m.sy2; return CE_OK;
    case SV_SUMXY: *out = m.sxy; return CE_OK;
    case SV_SUMX2Y: *out = m.sx2y; return CE_OK;
    case SV_YBAR: *out = m.ybar; return CE_OK;
    case SV_SIGX: CHK(num_div(&t, &m.Sxx, &m.n)); CHK(num_sqrt(out, &t)); return CE_OK;
    case SV_SIGY: CHK(num_div(&t, &m.Syy, &m.n)); CHK(num_sqrt(out, &t)); return CE_OK;
    case SV_SX:
        CHK(num_sub(&nm1, &m.n, &one));
        if (num_is_zero(&nm1)) return CE_MATH;
        CHK(num_div(&t, &m.Sxx, &nm1)); CHK(num_sqrt(out, &t)); return CE_OK;
    case SV_SY:
        CHK(num_sub(&nm1, &m.n, &one));
        if (num_is_zero(&nm1)) return CE_MATH;
        CHK(num_div(&t, &m.Syy, &nm1)); CHK(num_sqrt(out, &t)); return CE_OK;
    default: return CE_ARG;
    }
}

calc_err_t stat_yhat(const stat_t *s, const num_t *x, num_t *out)
{
    reg_t g;
    calc_err_t e = regress(s, &g);
    num_t t, u;
    dec_t d;
    if (e) return e;
    switch (s->type) {
    case ST_LIN: CHK(num_mul(&t, &g.b, x)); CHK(num_add(out, &g.a, &t)); return CE_OK;
    case ST_QUAD:
        CHK(num_mul(&t, &g.c, x)); CHK(num_add(&t, &t, &g.b)); CHK(num_mul(&t, &t, x)); CHK(num_add(out, &t, &g.a));
        return CE_OK;
    case ST_LOG: CHK(fn_ln(&d, &x->v)); num_from_dec(&t, &d); CHK(num_mul(&t, &t, &g.b)); CHK(num_add(out, &g.a, &t)); return CE_OK;
    case ST_EXP: CHK(num_mul(&t, &g.b, x)); CHK(fn_exp(&d, &t.v)); num_from_dec(&u, &d); CHK(num_mul(out, &g.a, &u)); return CE_OK;
    case ST_AB: CHK(num_pow(&t, &g.b, x)); CHK(num_mul(out, &g.a, &t)); return CE_OK;
    case ST_POW: CHK(num_pow(&t, x, &g.b)); CHK(num_mul(out, &g.a, &t)); return CE_OK;
    case ST_INV: { num_t one; num_from_int(&one, 1); CHK(num_div(&t, &one, x)); CHK(num_mul(&t, &t, &g.b)); CHK(num_add(out, &g.a, &t)); return CE_OK; }
    default: return CE_ARG;
    }
}

calc_err_t stat_xhat(const stat_t *s, const num_t *y, num_t *out)
{
    reg_t g;
    calc_err_t e = regress(s, &g);
    num_t t, u, one;
    dec_t d;
    if (e) return e;
    num_from_int(&one, 1);
    switch (s->type) {
    case ST_LIN: if (num_is_zero(&g.b)) return CE_MATH; CHK(num_sub(&t, y, &g.a)); CHK(num_div(out, &t, &g.b)); return CE_OK;
    case ST_LOG: if (num_is_zero(&g.b)) return CE_MATH; CHK(num_sub(&t, y, &g.a)); CHK(num_div(&t, &t, &g.b)); CHK(fn_exp(&d, &t.v)); num_from_dec(out, &d); return CE_OK;
    case ST_EXP: CHK(num_div(&t, y, &g.a)); CHK(fn_ln(&d, &t.v)); num_from_dec(&u, &d); CHK(num_div(out, &u, &g.b)); return CE_OK;
    case ST_AB: CHK(num_div(&t, y, &g.a)); CHK(fn_ln(&d, &t.v)); num_from_dec(&u, &d); CHK(fn_ln(&d, &g.b.v)); num_from_dec(&t, &d); CHK(num_div(out, &u, &t)); return CE_OK;
    case ST_POW: CHK(num_div(&t, y, &g.a)); CHK(num_div(&u, &one, &g.b)); CHK(num_pow(out, &t, &u)); return CE_OK;
    case ST_INV: CHK(num_sub(&t, y, &g.a)); if (num_is_zero(&t)) return CE_MATH; CHK(num_div(&t, &g.b, &t)); *out = t; return CE_OK;
    default: return CE_ARG;
    }
}
