#include "numeric/decimal.h"
#include "numeric/decimal_internal.h"

const char *dec_status_name(dec_status s)
{
    switch (s) {
    case DEC_OK:          return "OK";
    case DEC_UNDERFLOW:   return "UNDERFLOW";
    case DEC_OVERFLOW:    return "OVERFLOW";
    case DEC_DIV_BY_ZERO: return "DIV0";
    case DEC_DOMAIN:      return "DOMAIN";
    case DEC_PARSE:       return "PARSE";
    }
    return "?";
}

void dec_zero(dec_t *r)
{
    r->m = 0;
    r->exp = 0;
    r->neg = 0;
}

int dec_is_zero(const dec_t *a)
{
    return a->m == 0;
}

int dec_is_valid(const dec_t *a)
{
    if (a->m == 0) {
        return a->exp == 0 && a->neg == 0;
    }
    return a->m >= DEC_MANT_MIN && a->m < DEC_MANT_LIMIT &&
           a->exp >= DEC_EXP_MIN && a->exp <= DEC_EXP_MAX && a->neg <= 1;
}

int dec_cmp(const dec_t *a, const dec_t *b)
{
    int c;

    if (a->m == 0 && b->m == 0) {
        return 0;
    }
    if (a->m == 0) {
        return b->neg ? 1 : -1;
    }
    if (b->m == 0) {
        return a->neg ? -1 : 1;
    }
    if (a->neg != b->neg) {
        return a->neg ? -1 : 1;
    }
    if (a->exp != b->exp) {
        c = a->exp > b->exp ? 1 : -1;
    } else if (a->m != b->m) {
        c = a->m > b->m ? 1 : -1;
    } else {
        c = 0;
    }
    return a->neg ? -c : c;
}

void dec_neg(dec_t *r, const dec_t *a)
{
    *r = *a;
    if (r->m != 0) {
        r->neg = (uint8_t)(r->neg ^ 1u);
    }
}

void dec_abs(dec_t *r, const dec_t *a)
{
    *r = *a;
    r->neg = 0;
}

dec_status dec_pack_internal(dec_t *r, int neg, u128 n, int sticky, int32_t e)
{
    uint64_t q;
    int32_t sci;
    int nd;

    if (u128_is_zero(n)) {
        dec_zero(r);
        return sticky ? DEC_UNDERFLOW : DEC_OK;
    }

    nd = u128_ndigits(n);
    if (nd <= DEC_DIGITS) {
        /* Fewer digits than we keep: scale up, this is exact. */
        q = n.lo * u64_pow10[DEC_DIGITS - nd];
        e -= (DEC_DIGITS - nd);
    } else {
        int drop = nd - DEC_DIGITS;
        int k = drop - 1;
        int st = sticky;
        uint32_t round_digit;

        /* Discard all but the last dropped digit, remembering if any was non-zero. */
        while (k > 0) {
            int c = k > 9 ? 9 : k;
            uint32_t rem = u128_divmod_u32(&n, (uint32_t)u64_pow10[c]);
            if (rem != 0) {
                st = 1;
            }
            k -= c;
        }
        round_digit = u128_divmod_u32(&n, 10u);
        q = n.lo;

        /* Round half to even. */
        if (round_digit > 5 || (round_digit == 5 && (st || (q & 1u)))) {
            q++;
            if (q == DEC_MANT_LIMIT) {
                q = DEC_MANT_MIN;
                e++;
            }
        }
        e += drop;
    }

    sci = e + (DEC_DIGITS - 1);
    if (sci > DEC_EXP_MAX) {
        dec_zero(r);
        return DEC_OVERFLOW;
    }
    if (sci < DEC_EXP_MIN) {
        dec_zero(r);
        return DEC_UNDERFLOW;
    }

    r->m = q;
    r->exp = (int16_t)sci;
    r->neg = (uint8_t)(neg != 0);
    return DEC_OK;
}

dec_status dec_from_i64(dec_t *r, int64_t v)
{
    uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
    return dec_pack_internal(r, v < 0, u128_from_u64(mag), 0, 0);
}

dec_status dec_from_str(dec_t *r, const char *s)
{
    enum { MAX_KEPT = 34, EXP_CAP = 100000 };
    int neg = 0;
    int seen_digit = 0;
    int seen_dot = 0;
    int started = 0;
    int kept = 0;
    int sticky = 0;
    int32_t e = 0;
    int32_t ex = 0;
    u128 n = u128_from_u64(0);

    if (*s == '+' || *s == '-') {
        neg = (*s == '-');
        s++;
    }

    for (;; s++) {
        char c = *s;

        if (c >= '0' && c <= '9') {
            unsigned d = (unsigned)(c - '0');

            seen_digit = 1;
            if (started || d != 0) {
                if (kept < MAX_KEPT) {
                    n = u128_add(u128_mulw(n, 10u), u128_from_u64(d));
                    kept++;
                    started = 1;
                    if (seen_dot) {
                        e--;
                    }
                } else {
                    if (d != 0) {
                        sticky = 1;
                    }
                    if (!seen_dot) {
                        e++;
                    }
                }
            } else if (seen_dot) {
                e--; /* leading zero after the point */
            }
        } else if (c == '.') {
            if (seen_dot) {
                dec_zero(r);
                return DEC_PARSE;
            }
            seen_dot = 1;
        } else {
            break;
        }
    }

    if (!seen_digit) {
        dec_zero(r);
        return DEC_PARSE;
    }

    if (*s == 'e' || *s == 'E') {
        int eneg = 0;
        int edigits = 0;

        s++;
        if (*s == '+' || *s == '-') {
            eneg = (*s == '-');
            s++;
        }
        while (*s >= '0' && *s <= '9') {
            if (ex < EXP_CAP) {
                ex = ex * 10 + (*s - '0');
            }
            edigits++;
            s++;
        }
        if (edigits == 0) {
            dec_zero(r);
            return DEC_PARSE;
        }
        e += eneg ? -ex : ex;
    }

    if (*s != '\0') {
        dec_zero(r);
        return DEC_PARSE;
    }

    if (u128_is_zero(n)) {
        dec_zero(r);
        return DEC_OK;
    }
    return dec_pack_internal(r, neg, n, sticky, e);
}

size_t dec_to_str(const dec_t *a, char *buf, size_t cap)
{
    char digits[DEC_DIGITS];
    char *p = buf;
    uint64_t m;
    int e;
    int i;

    if (cap < DEC_STR_MAX) {
        if (cap > 0) {
            buf[0] = '\0';
        }
        return 0;
    }

    if (a->m == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    m = a->m;
    for (i = DEC_DIGITS - 1; i >= 0; i--) {
        digits[i] = (char)('0' + (int)(m % 10u));
        m /= 10u;
    }

    if (a->neg) {
        *p++ = '-';
    }
    *p++ = digits[0];
    *p++ = '.';
    for (i = 1; i < DEC_DIGITS; i++) {
        *p++ = digits[i];
    }
    *p++ = 'E';
    e = a->exp;
    if (e < 0) {
        *p++ = '-';
        e = -e;
    } else {
        *p++ = '+';
    }
    *p++ = (char)('0' + e / 10);
    *p++ = (char)('0' + e % 10);
    *p = '\0';
    return (size_t)(p - buf);
}
