#include "values/cplx.h"

#define CHK(x) do { dec_status _s = (x); if (DEC_STATUS_IS_ERROR(_s)) return _s; } while (0)

void cx_make(cx_t *r, const num_t *re, const num_t *im) { r->re = *re; r->im = *im; }
void cx_from_real(cx_t *r, const num_t *re) { r->re = *re; num_from_int(&r->im, 0); }
int cx_is_real(const cx_t *a) { return num_is_zero(&a->im); }

static void n_int(num_t *r, int64_t v) { num_from_int(r, v); }

dec_status cx_neg(cx_t *r, const cx_t *a)
{
    cx_t t;
    CHK(num_neg(&t.re, &a->re));
    CHK(num_neg(&t.im, &a->im));
    *r = t;
    return DEC_OK;
}

dec_status cx_conj(cx_t *r, const cx_t *a)
{
    cx_t t;
    t.re = a->re;
    CHK(num_neg(&t.im, &a->im));
    *r = t;
    return DEC_OK;
}

dec_status cx_add(cx_t *r, const cx_t *a, const cx_t *b)
{
    cx_t t;
    CHK(num_add(&t.re, &a->re, &b->re));
    CHK(num_add(&t.im, &a->im, &b->im));
    *r = t;
    return DEC_OK;
}

dec_status cx_sub(cx_t *r, const cx_t *a, const cx_t *b)
{
    cx_t t;
    CHK(num_sub(&t.re, &a->re, &b->re));
    CHK(num_sub(&t.im, &a->im, &b->im));
    *r = t;
    return DEC_OK;
}

dec_status cx_mul(cx_t *r, const cx_t *a, const cx_t *b)
{
    cx_t t;
    num_t p, q;
    CHK(num_mul(&p, &a->re, &b->re));
    CHK(num_mul(&q, &a->im, &b->im));
    CHK(num_sub(&t.re, &p, &q));
    CHK(num_mul(&p, &a->re, &b->im));
    CHK(num_mul(&q, &a->im, &b->re));
    CHK(num_add(&t.im, &p, &q));
    *r = t;
    return DEC_OK;
}

dec_status cx_div(cx_t *r, const cx_t *a, const cx_t *b)
{
    cx_t t;
    num_t den, p, q;
    if (cx_is_real(b)) {
        CHK(num_div(&t.re, &a->re, &b->re));
        CHK(num_div(&t.im, &a->im, &b->re));
        *r = t;
        return DEC_OK;
    }
    CHK(num_mul(&p, &b->re, &b->re));
    CHK(num_mul(&q, &b->im, &b->im));
    CHK(num_add(&den, &p, &q));
    CHK(num_mul(&p, &a->re, &b->re));
    CHK(num_mul(&q, &a->im, &b->im));
    CHK(num_add(&p, &p, &q));
    CHK(num_div(&t.re, &p, &den));
    CHK(num_mul(&p, &a->im, &b->re));
    CHK(num_mul(&q, &a->re, &b->im));
    CHK(num_sub(&p, &p, &q));
    CHK(num_div(&t.im, &p, &den));
    *r = t;
    return DEC_OK;
}

dec_status cx_abs(num_t *r, const cx_t *a)
{
    num_t p, q, s;
    if (num_is_zero(&a->im)) return num_abs(r, &a->re);
    if (num_is_zero(&a->re)) return num_abs(r, &a->im);
    CHK(num_mul(&p, &a->re, &a->re));
    CHK(num_mul(&q, &a->im, &a->im));
    CHK(num_add(&s, &p, &q));
    return num_sqrt(r, &s);
}

static void half_turn(num_t *r, ang_t mode)
{
    num_t pi;
    num_pi(&pi);
    (void)num_convert_angle(r, &pi, ANG_RAD, mode);
}

dec_status cx_arg(num_t *r, const cx_t *a, ang_t mode)
{
    num_t t, ht, q;
    int sre = dec_is_zero(&a->re.v) ? 0 : (a->re.v.neg ? -1 : 1);
    int sim = dec_is_zero(&a->im.v) ? 0 : (a->im.v.neg ? -1 : 1);

    if (sre == 0 && sim == 0) return DEC_DOMAIN;
    half_turn(&ht, mode);
    if (sre == 0) {
        n_int(&q, 2);
        CHK(num_div(&t, &ht, &q));
        if (sim > 0) { *r = t; return DEC_OK; }
        return num_neg(r, &t);
    }
    if (sim == 0) {
        if (sre > 0) { n_int(r, 0); return DEC_OK; }
        *r = ht;
        return DEC_OK;
    }
    CHK(num_div(&q, &a->im, &a->re));
    CHK(num_atan(&t, &q, mode));
    if (sre < 0) {
        if (sim > 0) CHK(num_add(&t, &t, &ht));
        else CHK(num_sub(&t, &t, &ht));
    }
    *r = t;
    return DEC_OK;
}

dec_status cx_polar(cx_t *r, const num_t *mag, const num_t *theta, ang_t mode)
{
    cx_t t;
    num_t c, s;
    CHK(num_cos(&c, theta, mode));
    CHK(num_sin(&s, theta, mode));
    CHK(num_mul(&t.re, mag, &c));
    CHK(num_mul(&t.im, mag, &s));
    *r = t;
    return DEC_OK;
}

dec_status cx_exp(cx_t *r, const cx_t *a)
{
    dec_t d;
    num_t m, mag;
    cx_t t;
    CHK(fn_exp(&d, &a->re.v));
    num_from_dec(&mag, &d);
    if (num_is_zero(&a->im)) {
        cx_from_real(r, &mag);
        return DEC_OK;
    }
    m = a->im;
    CHK(cx_polar(&t, &mag, &m, ANG_RAD));
    *r = t;
    return DEC_OK;
}

dec_status cx_ln(cx_t *r, const cx_t *a)
{
    cx_t t;
    num_t n2, p, q, half, ln2, arg;
    dec_t d;

    CHK(num_mul(&p, &a->re, &a->re));
    CHK(num_mul(&q, &a->im, &a->im));
    CHK(num_add(&n2, &p, &q));
    if (num_is_zero(&n2)) return DEC_DOMAIN;
    CHK(fn_ln(&d, &n2.v));
    num_from_dec(&ln2, &d);
    n_int(&half, 1);
    n_int(&q, 2);
    CHK(num_div(&half, &half, &q));
    CHK(num_mul(&t.re, &ln2, &half));
    CHK(cx_arg(&arg, a, ANG_RAD));
    t.im = arg;
    *r = t;
    return DEC_OK;
}

dec_status cx_log10(cx_t *r, const cx_t *a)
{
    cx_t l;
    dec_t d, ten;
    num_t n10;
    cx_t t10;
    CHK(cx_ln(&l, a));
    dec_from_i64(&ten, 10);
    CHK(fn_ln(&d, &ten));
    num_from_dec(&n10, &d);
    cx_from_real(&t10, &n10);
    return cx_div(r, &l, &t10);
}

dec_status cx_sqrt(cx_t *r, const cx_t *a)
{
    cx_t t;
    num_t mag, p, two, hi, lo;
    int neg_im = a->im.v.neg && !dec_is_zero(&a->im.v);

    n_int(&two, 2);
    if (num_is_zero(&a->im)) {
        if (!a->re.v.neg || dec_is_zero(&a->re.v)) {
            CHK(num_sqrt(&t.re, &a->re));
            n_int(&t.im, 0);
        } else {
            CHK(num_neg(&p, &a->re));
            n_int(&t.re, 0);
            CHK(num_sqrt(&t.im, &p));
        }
        *r = t;
        return DEC_OK;
    }
    CHK(cx_abs(&mag, a));
    CHK(num_add(&hi, &mag, &a->re));
    CHK(num_div(&hi, &hi, &two));
    CHK(num_sub(&lo, &mag, &a->re));
    CHK(num_div(&lo, &lo, &two));
    CHK(num_sqrt(&t.re, &hi));
    CHK(num_sqrt(&t.im, &lo));
    if (neg_im) CHK(num_neg(&t.im, &t.im));
    *r = t;
    return DEC_OK;
}

dec_status cx_pow(cx_t *r, const cx_t *a, const cx_t *b)
{
    cx_t t, l, w;
    int64_t n;

    if (cx_is_real(b) && dec_to_i64(&b->re.v, &n) == 0 && n >= -256 && n <= 256) {
        cx_t base = *a, acc;
        int64_t k = n < 0 ? -n : n;
        num_t one;
        n_int(&one, 1);
        cx_from_real(&acc, &one);
        if (n == 0) {
            if (num_is_zero(&a->re) && num_is_zero(&a->im)) return DEC_DOMAIN;
            *r = acc;
            return DEC_OK;
        }
        while (k) {
            if (k & 1) CHK(cx_mul(&acc, &acc, &base));
            k >>= 1;
            if (k) CHK(cx_mul(&base, &base, &base));
        }
        if (n < 0) {
            cx_t o;
            cx_from_real(&o, &one);
            CHK(cx_div(&acc, &o, &acc));
        }
        *r = acc;
        return DEC_OK;
    }
    if (num_is_zero(&a->re) && num_is_zero(&a->im)) {
        if (cx_is_real(b) && !b->re.v.neg && !dec_is_zero(&b->re.v)) {
            num_t z;
            n_int(&z, 0);
            cx_from_real(r, &z);
            return DEC_OK;
        }
        return DEC_DOMAIN;
    }
    CHK(cx_ln(&l, a));
    CHK(cx_mul(&w, &l, b));
    CHK(cx_exp(&t, &w));
    *r = t;
    return DEC_OK;
}

/* sin(a+bi) = sin a cosh b + i cos a sinh b (radians) */
dec_status cx_sin(cx_t *r, const cx_t *a)
{
    cx_t t;
    num_t s, c, ch, sh;
    dec_t d;
    CHK(num_sin(&s, &a->re, ANG_RAD));
    if (num_is_zero(&a->im)) { cx_from_real(r, &s); return DEC_OK; }
    CHK(num_cos(&c, &a->re, ANG_RAD));
    CHK(fn_cosh(&d, &a->im.v)); num_from_dec(&ch, &d);
    CHK(fn_sinh(&d, &a->im.v)); num_from_dec(&sh, &d);
    CHK(num_mul(&t.re, &s, &ch));
    CHK(num_mul(&t.im, &c, &sh));
    *r = t;
    return DEC_OK;
}

dec_status cx_cos(cx_t *r, const cx_t *a)
{
    cx_t t;
    num_t s, c, ch, sh;
    dec_t d;
    CHK(num_cos(&c, &a->re, ANG_RAD));
    if (num_is_zero(&a->im)) { cx_from_real(r, &c); return DEC_OK; }
    CHK(num_sin(&s, &a->re, ANG_RAD));
    CHK(fn_cosh(&d, &a->im.v)); num_from_dec(&ch, &d);
    CHK(fn_sinh(&d, &a->im.v)); num_from_dec(&sh, &d);
    CHK(num_mul(&t.re, &c, &ch));
    CHK(num_mul(&t.im, &s, &sh));
    CHK(num_neg(&t.im, &t.im));
    *r = t;
    return DEC_OK;
}

dec_status cx_tan(cx_t *r, const cx_t *a)
{
    cx_t s, c;
    CHK(cx_sin(&s, a));
    CHK(cx_cos(&c, a));
    return cx_div(r, &s, &c);
}

/* sinh(a+bi) = sinh a cos b + i cosh a sin b */
dec_status cx_sinh(cx_t *r, const cx_t *a)
{
    cx_t t;
    num_t s, c, ch, sh;
    dec_t d;
    CHK(fn_sinh(&d, &a->re.v)); num_from_dec(&sh, &d);
    if (num_is_zero(&a->im)) { cx_from_real(r, &sh); return DEC_OK; }
    CHK(fn_cosh(&d, &a->re.v)); num_from_dec(&ch, &d);
    CHK(num_cos(&c, &a->im, ANG_RAD));
    CHK(num_sin(&s, &a->im, ANG_RAD));
    CHK(num_mul(&t.re, &sh, &c));
    CHK(num_mul(&t.im, &ch, &s));
    *r = t;
    return DEC_OK;
}

dec_status cx_cosh(cx_t *r, const cx_t *a)
{
    cx_t t;
    num_t s, c, ch, sh;
    dec_t d;
    CHK(fn_cosh(&d, &a->re.v)); num_from_dec(&ch, &d);
    if (num_is_zero(&a->im)) { cx_from_real(r, &ch); return DEC_OK; }
    CHK(fn_sinh(&d, &a->re.v)); num_from_dec(&sh, &d);
    CHK(num_cos(&c, &a->im, ANG_RAD));
    CHK(num_sin(&s, &a->im, ANG_RAD));
    CHK(num_mul(&t.re, &ch, &c));
    CHK(num_mul(&t.im, &sh, &s));
    *r = t;
    return DEC_OK;
}

dec_status cx_tanh(cx_t *r, const cx_t *a)
{
    cx_t s, c;
    CHK(cx_sinh(&s, a));
    CHK(cx_cosh(&c, a));
    return cx_div(r, &s, &c);
}
