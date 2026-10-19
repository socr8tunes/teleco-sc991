#include "values/num.h"
#include "functions/xfunc.h"
#include "numeric/xconst.h"

/* ------------------------------------------------------------------------ */
/* construction                                                             */
/* ------------------------------------------------------------------------ */

void num_zero(num_t *r)
{
    dec_zero(&r->v);
    ex_int(&r->ex, 0);
}

void num_from_int(num_t *r, int64_t n)
{
    if (dec_from_i64(&r->v, n) != DEC_OK) {
        dec_zero(&r->v);
    }
    if (n == INT64_MIN) {
        ex_none(&r->ex);
    } else {
        ex_int(&r->ex, n);
    }
}

void num_from_dec(num_t *r, const dec_t *d)
{
    r->v = *d;
    ex_none(&r->ex);
}

dec_status num_from_ex(num_t *r, const ex_t *x)
{
    dec_status st = ex_to_dec(&r->v, x);

    if (DEC_STATUS_IS_ERROR(st)) {
        ex_none(&r->ex);
        return st;
    }
    r->ex = *x;
    return st;
}

void num_pi(num_t *r)
{
    ex_t p;
    ex_pi(&p, 1, 1);
    (void)num_from_ex(r, &p);
}

int num_is_zero(const num_t *a)
{
    return dec_is_zero(&a->v);
}

int num_is_exact(const num_t *a)
{
    return ex_valid(&a->ex);
}

int num_cmp(const num_t *a, const num_t *b)
{
    return dec_cmp(&a->v, &b->v);
}

void num_xn(xnum_t *x, const num_t *a)
{
    if (ex_valid(&a->ex)) {
        ex_to_xn(x, &a->ex);
    } else {
        xn_from_dec(x, &a->v);
    }
}

dec_status num_wrap(num_t *r, const dec_t *d, dec_status st)
{
    if (DEC_STATUS_IS_ERROR(st)) {
        num_zero(r);
        return st;
    }
    num_from_dec(r, d);
    return st;
}

/* ------------------------------------------------------------------------ */
/* arithmetic                                                               */
/* ------------------------------------------------------------------------ */

typedef int (*ex_op2)(ex_t *, const ex_t *, const ex_t *);
typedef dec_status (*dec_op2)(dec_t *, const dec_t *, const dec_t *);
typedef void (*xn_op2)(xnum_t *, const xnum_t *, const xnum_t *);

/*
 * Decimal operation first (for error detection). If either operand has an exact
 * form, try an exact result; failing that, redo the operation on the full-precision
 * values so that exact operands such as pi are never rounded twice.
 */
static dec_status binop(num_t *r, const num_t *a, const num_t *b, dec_op2 dop, ex_op2 eop, xn_op2 xop)
{
    num_t t;
    dec_status st = dop(&t.v, &a->v, &b->v);

    ex_none(&t.ex);
    if (DEC_STATUS_IS_ERROR(st)) {
        num_zero(r);
        return st;
    }
    if (ex_valid(&a->ex) || ex_valid(&b->ex)) {
        ex_t e;
        dec_t ev;
        dec_status st2;

        if (ex_valid(&a->ex) && ex_valid(&b->ex) && eop(&e, &a->ex, &b->ex) == 0) {
            st2 = ex_to_dec(&ev, &e);
            if (!DEC_STATUS_IS_ERROR(st2)) {
                t.ex = e;
                t.v = ev;
                st = st2;
            }
        } else {
            xnum_t xa, xb, xr;
            num_xn(&xa, a);
            num_xn(&xb, b);
            xop(&xr, &xa, &xb);
            st2 = xn_to_dec(&ev, &xr);
            if (!DEC_STATUS_IS_ERROR(st2)) {
                t.v = ev;
                st = st2;
            }
        }
    }
    *r = t;
    return st;
}

dec_status num_add(num_t *r, const num_t *a, const num_t *b)
{
    return binop(r, a, b, dec_add, ex_add, xn_add);
}

dec_status num_sub(num_t *r, const num_t *a, const num_t *b)
{
    return binop(r, a, b, dec_sub, ex_sub, xn_sub);
}

dec_status num_mul(num_t *r, const num_t *a, const num_t *b)
{
    return binop(r, a, b, dec_mul, ex_mul, xn_mul);
}

dec_status num_div(num_t *r, const num_t *a, const num_t *b)
{
    return binop(r, a, b, dec_div, ex_div, xn_div);
}

dec_status num_neg(num_t *r, const num_t *a)
{
    num_t t = *a;

    dec_neg(&t.v, &a->v);
    if (ex_valid(&a->ex) && ex_neg(&t.ex, &a->ex) != 0) {
        ex_none(&t.ex);
    }
    *r = t;
    return DEC_OK;
}

dec_status num_abs(num_t *r, const num_t *a)
{
    if (a->v.neg) {
        return num_neg(r, a);
    }
    *r = *a;
    return DEC_OK;
}

dec_status num_sqrt(num_t *r, const num_t *a)
{
    num_t t;
    dec_status st = dec_sqrt(&t.v, &a->v);

    ex_none(&t.ex);
    if (DEC_STATUS_IS_ERROR(st)) {
        num_zero(r);
        return st;
    }
    if (ex_valid(&a->ex)) {
        ex_t e;
        dec_t ev;
        if (ex_sqrt(&e, &a->ex) == 0 && !DEC_STATUS_IS_ERROR(ex_to_dec(&ev, &e))) {
            t.ex = e;
            t.v = ev;
        } else {
            xnum_t xa, xr;
            num_xn(&xa, a);
            xn_sqrt(&xr, &xa);
            if (!DEC_STATUS_IS_ERROR(xn_to_dec(&ev, &xr))) {
                t.v = ev;
            }
        }
    }
    *r = t;
    return st;
}

dec_status num_pow(num_t *r, const num_t *a, const num_t *b)
{
    num_t t;
    dec_status st;
    int64_t n;

    /* negative base with a rational exponent p/q, q odd: real root */
    if (a->v.neg && ex_valid(&b->ex) && ex_is_rational(&b->ex) && b->ex.d != 1 && (b->ex.d & 1)) {
        num_t root, base_n;
        dec_t nd;
        int64_t q = b->ex.d, p = b->ex.a;
        num_t qn;

        num_from_int(&qn, q);
        st = num_root(&root, &qn, a);
        if (DEC_STATUS_IS_ERROR(st)) {
            num_zero(r);
            return st;
        }
        (void)nd;
        num_from_int(&base_n, p);
        return num_pow(r, &root, &base_n);
    }

    {
        xnum_t xa, xb;
        num_xn(&xa, a);
        num_xn(&xb, b);
        st = fx_pow(&t.v, &xa, &xb);
    }
    ex_none(&t.ex);
    if (DEC_STATUS_IS_ERROR(st)) {
        num_zero(r);
        return st;
    }
    if (ex_valid(&a->ex) && ex_valid(&b->ex) && ex_is_integer(&b->ex)) {
        n = b->ex.a;
        if (n >= -64 && n <= 64) {
            ex_t e;
            if (ex_powi(&e, &a->ex, n) == 0) {
                dec_t ev;
                if (!DEC_STATUS_IS_ERROR(ex_to_dec(&ev, &e))) {
                    t.ex = e;
                    t.v = ev;
                }
            }
        }
    } else if (ex_valid(&a->ex) && ex_valid(&b->ex) && ex_is_rational(&b->ex) && ex_is_rational(&a->ex)) {
        /* a^(p/q) exact when a is a perfect q-th power; q == 2 may give a surd */
        ex_t e, r1;
        int ok = -1;
        if (b->ex.d == 2) {
            if (ex_sqrt(&r1, &a->ex) == 0) {
                ok = ex_powi(&e, &r1, b->ex.a);
            }
        } else if (ex_iroot(&r1, &a->ex, b->ex.d) == 0) {
            ok = ex_powi(&e, &r1, b->ex.a);
        }
        if (ok == 0) {
            dec_t ev;
            if (!DEC_STATUS_IS_ERROR(ex_to_dec(&ev, &e))) {
                t.ex = e;
                t.v = ev;
            }
        }
    }
    *r = t;
    return st;
}

dec_status num_root(num_t *r, const num_t *n, const num_t *a)
{
    num_t t;
    dec_status st;

    if (ex_valid(&n->ex) && ex_is_integer(&n->ex) && n->ex.a == 2) {
        return num_sqrt(r, a);
    }
    {
        xnum_t xn_, xa;
        num_xn(&xn_, n);
        num_xn(&xa, a);
        st = fx_xroot(&t.v, &xn_, &xa);
    }
    ex_none(&t.ex);
    if (DEC_STATUS_IS_ERROR(st)) {
        num_zero(r);
        return st;
    }
    if (ex_valid(&n->ex) && ex_is_integer(&n->ex) && ex_valid(&a->ex) && n->ex.a >= 2) {
        ex_t e;
        if (ex_iroot(&e, &a->ex, n->ex.a) == 0) {
            dec_t ev;
            if (!DEC_STATUS_IS_ERROR(ex_to_dec(&ev, &e))) {
                t.ex = e;
                t.v = ev;
            }
        }
    }
    *r = t;
    return st;
}

/* ------------------------------------------------------------------------ */
/* trig with exact special angles                                           */
/* ------------------------------------------------------------------------ */

static dec_status trig_common(num_t *r, const num_t *a, ang_t m, int fn)
{
    num_t t;
    dec_status st;

    if (ex_valid(&a->ex)) {
        ex_t e;
        int rc = ex_trig(fn, &a->ex, (int)m == (int)ANG_DEG ? 0 : (m == ANG_RAD ? 1 : 2), &e);
        if (rc == -2) {
            num_zero(r);
            return DEC_DOMAIN;
        }
        if (rc == 0) {
            return num_from_ex(r, &e);
        }
    }
    {
        xnum_t x;
        num_xn(&x, a);
        if (fn == 0) {
            st = fx_sin(&t.v, &x, m);
        } else if (fn == 1) {
            st = fx_cos(&t.v, &x, m);
        } else {
            st = fx_tan(&t.v, &x, m);
        }
    }
    return num_wrap(r, &t.v, st);
}

dec_status num_sin(num_t *r, const num_t *a, ang_t m) { return trig_common(r, a, m, 0); }
dec_status num_cos(num_t *r, const num_t *a, ang_t m) { return trig_common(r, a, m, 1); }
dec_status num_tan(num_t *r, const num_t *a, ang_t m) { return trig_common(r, a, m, 2); }

static dec_status itrig_common(num_t *r, const num_t *a, ang_t m, int fn)
{
    dec_t d;
    dec_status st;

    if (ex_valid(&a->ex)) {
        ex_t e;
        if (ex_itrig(fn, &a->ex, (int)m == (int)ANG_DEG ? 0 : (m == ANG_RAD ? 1 : 2), &e) == 0) {
            return num_from_ex(r, &e);
        }
    }
    {
        xnum_t x;
        num_xn(&x, a);
        st = fn == 0 ? fx_asin(&d, &x, m) : (fn == 1 ? fx_acos(&d, &x, m) : fx_atan(&d, &x, m));
    }
    return num_wrap(r, &d, st);
}

dec_status num_asin(num_t *r, const num_t *a, ang_t m) { return itrig_common(r, a, m, 0); }
dec_status num_acos(num_t *r, const num_t *a, ang_t m) { return itrig_common(r, a, m, 1); }
dec_status num_atan(num_t *r, const num_t *a, ang_t m) { return itrig_common(r, a, m, 2); }

dec_status num_convert_angle(num_t *r, const num_t *a, ang_t from, ang_t to)
{
    xnum_t x, t, u;
    dec_t d;
    dec_status st;

    if (from == to) {
        *r = *a;
        return DEC_OK;
    }
    if (ex_valid(&a->ex)) {
        ex_t deg, out;
        int have_deg = 0;
        int dmode = to == ANG_DEG ? 0 : (to == ANG_RAD ? 1 : 2);

        if (ex_is_rational(&a->ex) && a->ex.a > -1000000000000LL && a->ex.a < 1000000000000LL) {
            if (from == ANG_DEG) {
                deg = a->ex;
                have_deg = 1;
            } else if (from == ANG_GRAD) {
                have_deg = ex_rat(&deg, a->ex.a * 9, a->ex.d * 10) == 0;
            }
        } else if (a->ex.kind == EX_PI && from == ANG_RAD &&
                   a->ex.a > -1000000000000LL && a->ex.a < 1000000000000LL) {
            have_deg = ex_rat(&deg, a->ex.a * 180, a->ex.d) == 0;   /* (a/d) pi rad = 180 a/d deg */
        }
        if (have_deg && ex_deg_to_mode(&deg, dmode, &out) == 0) {
            return num_from_ex(r, &out);
        }
    }
    num_xn(&x, a);
    if (from == ANG_DEG) {
        xn_mul(&t, &x, &XC_RAD_PER_DEG);
    } else if (from == ANG_GRAD) {
        xn_mul(&t, &x, &XC_RAD_PER_GRAD);
    } else {
        t = x;
    }
    xf_rad_to_unit(&u, &t, to == ANG_DEG ? XF_UNIT_DEG : (to == ANG_GRAD ? XF_UNIT_GRAD : XF_UNIT_RAD));
    st = xn_to_dec(&d, &u);
    return num_wrap(r, &d, st);
}

/* ------------------------------------------------------------------------ */
/* combinatorics: exact integers when they fit                              */
/* ------------------------------------------------------------------------ */

static void mark_integer(num_t *r)
{
    int64_t n;

    if (dec_to_i64(&r->v, &n) == 0 && n > -9007199254740992LL && n < 9007199254740992LL
        && r->v.exp < 15) {
        ex_int(&r->ex, n);
    }
}

dec_status num_fact(num_t *r, const num_t *a)
{
    dec_t d;
    dec_status st = fn_fact(&d, &a->v);

    if (num_wrap(r, &d, st) == st && !DEC_STATUS_IS_ERROR(st)) {
        mark_integer(r);
    }
    return st;
}

dec_status num_npr(num_t *r, const num_t *n, const num_t *k)
{
    dec_t d;
    dec_status st = fn_npr(&d, &n->v, &k->v);

    if (num_wrap(r, &d, st) == st && !DEC_STATUS_IS_ERROR(st)) {
        mark_integer(r);
    }
    return st;
}

dec_status num_ncr(num_t *r, const num_t *n, const num_t *k)
{
    dec_t d;
    dec_status st = fn_ncr(&d, &n->v, &k->v);

    if (num_wrap(r, &d, st) == st && !DEC_STATUS_IS_ERROR(st)) {
        mark_integer(r);
    }
    return st;
}
