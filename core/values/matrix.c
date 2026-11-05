#include <string.h>
#include "values/matrix.h"

#define CHK(x) do { dec_status _s = (x); if (DEC_STATUS_IS_ERROR(_s)) return _s; } while (0)

static void set_dims(mv_t *o, int r, int c) { o->r = (uint8_t)r; o->c = (uint8_t)c; }

static dec_status ew(mv_t *out, const mv_t *a, const mv_t *b, dec_status (*op)(num_t *, const num_t *, const num_t *))
{
    int i, n;
    if (a->r != b->r || a->c != b->c) return DEC_DOMAIN;
    n = a->r * a->c;
    for (i = 0; i < n; i++) CHK(op(&out->e[i], &a->e[i], &b->e[i]));
    set_dims(out, a->r, a->c);
    return DEC_OK;
}

dec_status mx_add(mv_t *out, const mv_t *a, const mv_t *b) { return ew(out, a, b, num_add); }
dec_status mx_sub(mv_t *out, const mv_t *a, const mv_t *b) { return ew(out, a, b, num_sub); }

dec_status mx_mul(mv_t *out, const mv_t *a, const mv_t *b)
{
    int i, j, k;
    num_t t[MAT_MAX * MAT_MAX];
    if (a->c != b->r) return DEC_DOMAIN;
    for (i = 0; i < a->r; i++)
        for (j = 0; j < b->c; j++) {
            num_t acc, p;
            num_from_int(&acc, 0);
            for (k = 0; k < a->c; k++) {
                CHK(num_mul(&p, &a->e[i * a->c + k], &b->e[k * b->c + j]));
                CHK(num_add(&acc, &acc, &p));
            }
            t[i * b->c + j] = acc;
        }
    {
        int r = a->r, c = b->c;
        memcpy(out->e, t, (size_t)(r * c) * sizeof(num_t));
        set_dims(out, r, c);
    }
    return DEC_OK;
}

dec_status mx_scale(mv_t *out, const mv_t *a, const num_t *k)
{
    int i, n = a->r * a->c;
    for (i = 0; i < n; i++) CHK(num_mul(&out->e[i], &a->e[i], k));
    set_dims(out, a->r, a->c);
    return DEC_OK;
}

dec_status mx_div_scalar(mv_t *out, const mv_t *a, const num_t *k)
{
    int i, n = a->r * a->c;
    for (i = 0; i < n; i++) CHK(num_div(&out->e[i], &a->e[i], k));
    set_dims(out, a->r, a->c);
    return DEC_OK;
}

dec_status mx_neg(mv_t *out, const mv_t *a)
{
    int i, n = a->r * a->c;
    for (i = 0; i < n; i++) CHK(num_neg(&out->e[i], &a->e[i]));
    set_dims(out, a->r, a->c);
    return DEC_OK;
}

dec_status mx_trn(mv_t *out, const mv_t *a)
{
    int i, j;
    for (i = 0; i < a->r; i++)
        for (j = 0; j < a->c; j++) out->e[j * a->r + i] = a->e[i * a->c + j];
    set_dims(out, a->c, a->r);
    return DEC_OK;
}

dec_status mx_identity(mv_t *out, int n)
{
    int i, j;
    if (n < 1 || n > MAT_MAX) return DEC_DOMAIN;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) num_from_int(&out->e[i * n + j], i == j);
    set_dims(out, n, n);
    return DEC_OK;
}

/* choose pivot: first non-zero when entries are exact, else the largest magnitude */
static int pick_pivot(num_t *m, int n, int col, int from)
{
    int i, best = -1;
    for (i = from; i < n; i++) {
        if (num_is_zero(&m[i * n + col])) continue;
        if (best < 0) { best = i; if (num_is_exact(&m[i * n + col])) return i; continue; }
        {
            num_t a, b;
            num_abs(&a, &m[i * n + col]);
            num_abs(&b, &m[best * n + col]);
            if (num_cmp(&a, &b) > 0) best = i;
        }
    }
    return best;
}

dec_status mx_det(num_t *out, const mv_t *a)
{
    num_t m[MAT_MAX * MAT_MAX], det, f, p;
    int n = a->r, i, j, k, sign = 1;
    if (a->r != a->c) return DEC_DOMAIN;
    memcpy(m, a->e, (size_t)(n * n) * sizeof(num_t));
    num_from_int(&det, 1);
    for (k = 0; k < n; k++) {
        int piv = pick_pivot(m, n, k, k);
        if (piv < 0) { num_from_int(out, 0); return DEC_OK; }
        if (piv != k) {
            for (j = 0; j < n; j++) { num_t t = m[k * n + j]; m[k * n + j] = m[piv * n + j]; m[piv * n + j] = t; }
            sign = -sign;
        }
        CHK(num_mul(&det, &det, &m[k * n + k]));
        for (i = k + 1; i < n; i++) {
            CHK(num_div(&f, &m[i * n + k], &m[k * n + k]));
            for (j = k; j < n; j++) {
                CHK(num_mul(&p, &f, &m[k * n + j]));
                CHK(num_sub(&m[i * n + j], &m[i * n + j], &p));
            }
        }
    }
    if (sign < 0) CHK(num_neg(&det, &det));
    *out = det;
    return DEC_OK;
}

dec_status mx_inv(mv_t *out, const mv_t *a)
{
    num_t m[MAT_MAX * MAT_MAX], inv[MAT_MAX * MAT_MAX], f, p, d;
    int n = a->r, i, j, k;
    if (a->r != a->c) return DEC_DOMAIN;
    memcpy(m, a->e, (size_t)(n * n) * sizeof(num_t));
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) num_from_int(&inv[i * n + j], i == j);
    for (k = 0; k < n; k++) {
        int piv = pick_pivot(m, n, k, k);
        if (piv < 0) return DEC_DOMAIN;           /* singular */
        if (piv != k) {
            for (j = 0; j < n; j++) {
                num_t t = m[k * n + j]; m[k * n + j] = m[piv * n + j]; m[piv * n + j] = t;
                t = inv[k * n + j]; inv[k * n + j] = inv[piv * n + j]; inv[piv * n + j] = t;
            }
        }
        d = m[k * n + k];
        for (j = 0; j < n; j++) {
            CHK(num_div(&m[k * n + j], &m[k * n + j], &d));
            CHK(num_div(&inv[k * n + j], &inv[k * n + j], &d));
        }
        for (i = 0; i < n; i++) {
            if (i == k) continue;
            f = m[i * n + k];
            if (num_is_zero(&f)) continue;
            for (j = 0; j < n; j++) {
                CHK(num_mul(&p, &f, &m[k * n + j]));
                CHK(num_sub(&m[i * n + j], &m[i * n + j], &p));
                CHK(num_mul(&p, &f, &inv[k * n + j]));
                CHK(num_sub(&inv[i * n + j], &inv[i * n + j], &p));
            }
        }
    }
    memcpy(out->e, inv, (size_t)(n * n) * sizeof(num_t));
    set_dims(out, n, n);
    return DEC_OK;
}

dec_status mx_dot(num_t *out, const mv_t *a, const mv_t *b)
{
    int i;
    num_t acc, p;
    if (a->r != b->r || a->c != 1 || b->c != 1) return DEC_DOMAIN;
    num_from_int(&acc, 0);
    for (i = 0; i < a->r; i++) {
        CHK(num_mul(&p, &a->e[i], &b->e[i]));
        CHK(num_add(&acc, &acc, &p));
    }
    *out = acc;
    return DEC_OK;
}

dec_status mx_cross(mv_t *out, const mv_t *a, const mv_t *b)
{
    num_t t[3], p, q;
    int i;
    if (a->r != 3 || b->r != 3 || a->c != 1 || b->c != 1) return DEC_DOMAIN;
    for (i = 0; i < 3; i++) {
        int j = (i + 1) % 3, k = (i + 2) % 3;
        CHK(num_mul(&p, &a->e[j], &b->e[k]));
        CHK(num_mul(&q, &a->e[k], &b->e[j]));
        CHK(num_sub(&t[i], &p, &q));
    }
    memcpy(out->e, t, sizeof t);
    set_dims(out, 3, 1);
    return DEC_OK;
}

dec_status mx_norm(num_t *out, const mv_t *a)
{
    num_t s;
    CHK(mx_dot(&s, a, a));
    return num_sqrt(out, &s);
}

dec_status mx_angle(num_t *out, const mv_t *a, const mv_t *b, ang_t mode)
{
    num_t d, na, nb, den, c;
    CHK(mx_dot(&d, a, b));
    CHK(mx_norm(&na, a));
    CHK(mx_norm(&nb, b));
    CHK(num_mul(&den, &na, &nb));
    if (num_is_zero(&den)) return DEC_DIV_BY_ZERO;     /* angle with a zero vector: undefined (Math ERROR, not a dimension error) */
    CHK(num_div(&c, &d, &den));
    return num_acos(out, &c, mode);
}

dec_status mx_unit(mv_t *out, const mv_t *a)
{
    num_t n;
    CHK(mx_norm(&n, a));
    if (num_is_zero(&n)) return DEC_DOMAIN;
    return mx_div_scalar(out, a, &n);
}
