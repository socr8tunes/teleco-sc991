#include "functions/dist.h"
#include "functions/xfunc.h"
#include "numeric/xconst.h"

#define CHK(x) do { dec_status _s = (x); if (DEC_STATUS_IS_ERROR(_s)) return _s; } while (0)

/* upper tail R(|t|), full precision */
static dec_status tail_of(xnum_t *tail, const xnum_t *at)
{
    xnum_t two_pi, sq2pi, half, tmp, q, u, s, eps, k, term, lim;
    int kk;

    xn_mul_u32(&two_pi, &XC_PI, 2);
    xn_sqrt(&sq2pi, &two_pi);
    xn_from_i64(&half, 1);
    xn_div_u32(&half, &half, 2);
    xn_from_i64(&lim, 3);

    if (xn_cmp_abs(at, &lim) <= 0) {
        xn_mul(&q, at, at);
        xn_div_u32(&q, &q, 2);
        xn_from_i64(&u, 1);
        xn_from_i64(&s, 1);
        xn_pow10(&eps, -52);
        for (kk = 1; kk < 2000; kk++) {
            xn_mul(&u, &u, &q);
            xn_div_u32(&u, &u, (uint32_t)kk);
            u.neg = (uint8_t)!u.neg;
            xn_div_u32(&term, &u, (uint32_t)(2 * kk + 1));
            xn_add(&s, &s, &term);
            xn_from_i64(&k, kk);
            if (xn_cmp_abs(&u, &eps) < 0 && xn_cmp(&k, &q) > 0) break;
        }
        xn_mul(&tmp, at, &s);
        xn_div(&tmp, &tmp, &sq2pi);
        xn_sub(tail, &half, &tmp);
        return DEC_OK;
    }
    {
        xnum_t b, kx, e, ph;
        xn_mul(&q, at, at);
        xn_div_u32(&q, &q, 2);
        xn_from_i64(&lim, 700);
        if (xn_cmp(&q, &lim) > 0) { xn_zero(tail); return DEC_OK; }
        b = *at;
        for (kk = 300; kk >= 1; kk--) {
            xn_from_i64(&kx, kk);
            xn_div(&kx, &kx, &b);
            xn_add(&b, at, &kx);
        }
        xn_neg(&q, &q);
        xf_exp(&e, &q);
        xn_div(&ph, &e, &sq2pi);
        xn_div(tail, &ph, &b);
        return DEC_OK;
    }
}

dec_status dist_norm(int which, dec_t *r, const dec_t *t)
{
    xnum_t x, at, tail, one, half, res;
    int neg;

    xn_from_dec(&x, t);
    if (xn_is_zero(&x)) {
        if (which == NORM_Q) { dec_zero(r); return DEC_OK; }
        return dec_from_str(r, "0.5");
    }
    neg = x.neg;
    at = x;
    at.neg = 0;
    CHK(tail_of(&tail, &at));
    xn_from_i64(&one, 1);
    xn_from_i64(&half, 1);
    xn_div_u32(&half, &half, 2);
    switch (which) {
    case NORM_P: if (neg) res = tail; else xn_sub(&res, &one, &tail); break;
    case NORM_R: if (neg) xn_sub(&res, &one, &tail); else res = tail; break;
    default:
        xn_sub(&res, &half, &tail);       /* area from 0 to |t| */
        res.neg = (uint8_t)(neg && !xn_is_zero(&res));
        break;
    }
    if (xn_is_zero(&res)) { dec_zero(r); return DEC_OK; }
    return xn_to_dec(r, &res);
}
