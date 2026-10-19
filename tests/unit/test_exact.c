/* Unit tests for core/values (exact.c, num.c). */
#include <stdio.h>
#include <string.h>

#include "values/num.h"

static int g_checks, g_fail;

static void text(const num_t *n, char *buf, size_t cap)
{
    ex_text_t t;
    if (!ex_valid(&n->ex)) {
        snprintf(buf, cap, "~");
        return;
    }
    ex_format(&n->ex, 1, &t);
    if (t.den[0] && (strchr(t.num + 1, '+') || strchr(t.num + 1, '-'))) {
        snprintf(buf, cap, "%s(%s)/%s", t.neg ? "-" : "", t.num, t.den);
    } else {
        snprintf(buf, cap, "%s%s%s%s", t.neg ? "-" : "", t.num, t.den[0] ? "/" : "", t.den);
    }
}

static void expect(const char *what, const num_t *n, const char *want, const char *want_dec)
{
    char got[96], dec[DEC_STR_MAX];

    g_checks++;
    text(n, got, sizeof got);
    dec_to_str(&n->v, dec, sizeof dec);
    if (strcmp(got, want) != 0 || (want_dec && strcmp(dec, want_dec) != 0)) {
        g_fail++;
        printf("FAIL %s: got %s [%s], want %s [%s]\n", what, got, dec, want, want_dec ? want_dec : "-");
    }
}

static num_t I(int64_t v) { num_t n; num_from_int(&n, v); return n; }

static void test_rational(void)
{
    num_t r, a = I(1), b = I(3), c = I(6), t;

    num_div(&t, &a, &b);
    num_div(&r, &a, &c);
    num_add(&r, &t, &r);
    expect("1/3+1/6", &r, "1/2", "5.000000000000000E-01");

    num_div(&r, &a, &b);
    expect("1/3", &r, "1/3", "3.333333333333333E-01");

    num_mul(&r, &r, &b);
    expect("(1/3)*3", &r, "1", "1.000000000000000E+00");

    num_sub(&r, &a, &c);
    expect("1-6", &r, "-5", "-5.000000000000000E+00");

    {   /* overflow of int64 exact form falls back to decimal only */
        num_t big, q;
        num_from_int(&big, 4611686018427387904LL);   /* 2^62 */
        num_mul(&q, &big, &big);
        expect("2^62*2^62 not exact", &q, "~", NULL);
    }
    {   /* exact + inexact = inexact */
        num_t x, y;
        dec_t d;
        dec_from_str(&d, "0.5");
        num_from_dec(&x, &d);
        y = I(1);
        num_add(&r, &x, &y);
        expect("0.5+1 inexact", &r, "~", "1.500000000000000E+00");
    }
}

static void test_surds(void)
{
    num_t r, t, two = I(2), three = I(3), eight = I(8), twelve = I(12), one = I(1), five = I(5);

    num_sqrt(&r, &eight);
    expect("sqrt(8)", &r, "2sqrt(2)", "2.828427124746190E+00");
    num_sqrt(&r, &twelve);
    expect("sqrt(12)", &r, "2sqrt(3)", "3.464101615137755E+00");
    num_sqrt(&t, &two);
    num_mul(&r, &t, &t);
    expect("sqrt2*sqrt2", &r, "2", "2.000000000000000E+00");
    {
        num_t s3;
        num_sqrt(&s3, &three);
        num_mul(&r, &t, &s3);
        expect("sqrt2*sqrt3", &r, "sqrt(6)", "2.449489742783178E+00");
    }
    /* golden ratio (1+sqrt5)/2 */
    num_sqrt(&t, &five);
    num_add(&t, &one, &t);
    num_div(&r, &t, &two);
    expect("golden", &r, "(1+sqrt(5))/2", "1.618033988749895E+00");
    /* 1/(1+sqrt2) = sqrt2 - 1 */
    num_sqrt(&t, &two);
    num_add(&t, &one, &t);
    num_div(&r, &one, &t);
    expect("1/(1+sqrt2)", &r, "-1+sqrt(2)", "4.142135623730950E-01");
    /* sqrt(1/4)=1/2, sqrt(2/9)=sqrt2/3 */
    {
        num_t q = I(1), four = I(4), nine = I(9);
        num_div(&q, &q, &four);
        num_sqrt(&r, &q);
        expect("sqrt(1/4)", &r, "1/2", NULL);
        num_div(&q, &two, &nine);
        num_sqrt(&r, &q);
        expect("sqrt(2/9)", &r, "sqrt(2)/3", NULL);
    }
    /* mixed radicands cannot be added exactly */
    {
        num_t s2, s3;
        num_sqrt(&s2, &two);
        num_sqrt(&s3, &three);
        num_add(&r, &s2, &s3);
        expect("sqrt2+sqrt3 decimal only", &r, "~", "3.146264369941972E+00");
    }
    /* sqrt of a non-rational stays decimal */
    {
        num_t s2, s4;
        num_sqrt(&s2, &two);
        num_sqrt(&s4, &s2);
        expect("sqrt(sqrt2) decimal only", &s4, "~", "1.189207115002721E+00");
    }
}

static void test_pi(void)
{
    num_t r, p, six = I(6), two = I(2), t;

    num_pi(&p);
    expect("pi", &p, "pi", "3.141592653589793E+00");
    num_div(&r, &p, &six);
    expect("pi/6", &r, "pi/6", "5.235987755982989E-01");
    num_mul(&r, &p, &two);
    expect("2pi", &r, "2pi", "6.283185307179586E+00");
    num_div(&t, &r, &p);
    expect("2pi/pi", &t, "2", NULL);
    num_mul(&r, &p, &p);
    expect("pi*pi decimal only", &r, "~", "9.869604401089359E+00");
}

static void test_trig(void)
{
    num_t r, p, a, six = I(6), four = I(4), three = I(3), two = I(2), thirty = I(30), neg;

    num_pi(&p);
    num_div(&a, &p, &six);
    num_sin(&r, &a, ANG_RAD);
    expect("sin(pi/6)", &r, "1/2", "5.000000000000000E-01");
    num_sin(&r, &p, ANG_RAD);
    expect("sin(pi)", &r, "0", "0");
    num_cos(&r, &p, ANG_RAD);
    expect("cos(pi)", &r, "-1", "-1.000000000000000E+00");
    num_div(&a, &p, &four);
    num_sin(&r, &a, ANG_RAD);
    expect("sin(pi/4)", &r, "sqrt(2)/2", "7.071067811865475E-01");
    num_tan(&r, &a, ANG_RAD);
    expect("tan(pi/4)", &r, "1", "1.000000000000000E+00");
    num_div(&a, &p, &three);
    num_tan(&r, &a, ANG_RAD);
    expect("tan(pi/3)", &r, "sqrt(3)", "1.732050807568877E+00");
    num_cos(&r, &a, ANG_RAD);
    expect("cos(pi/3)", &r, "1/2", "5.000000000000000E-01");
    num_div(&a, &p, &two);
    num_tan(&r, &a, ANG_RAD) == DEC_DOMAIN ? (void)0 : (void)(g_fail++, printf("FAIL tan(pi/2) must be domain error\n"));
    g_checks++;

    num_sin(&r, &thirty, ANG_DEG);
    expect("sin(30deg)", &r, "1/2", "5.000000000000000E-01");
    num_neg(&neg, &thirty);
    num_cos(&r, &neg, ANG_DEG);
    expect("cos(-30deg)", &r, "sqrt(3)/2", "8.660254037844386E-01");
    {
        num_t h = I(1), s;
        num_div(&h, &h, &two);
        num_asin(&s, &h, ANG_DEG);
        expect("asin(1/2) deg", &s, "30", "3.000000000000000E+01");
        num_asin(&s, &h, ANG_RAD);
        expect("asin(1/2) rad", &s, "pi/6", "5.235987755982989E-01");
        num_acos(&s, &h, ANG_DEG);
        expect("acos(1/2) deg", &s, "60", NULL);
        num_neg(&h, &h);
        num_acos(&s, &h, ANG_DEG);
        expect("acos(-1/2) deg", &s, "120", NULL);
        num_asin(&s, &h, ANG_GRAD);
        expect("asin(-1/2) grad", &s, "-100/3", NULL);
    }
    {
        num_t s3, s;
        num_sqrt(&s3, &three);
        num_atan(&s, &s3, ANG_DEG);
        expect("atan(sqrt3) deg", &s, "60", NULL);
        num_atan(&s, &s3, ANG_RAD);
        expect("atan(sqrt3) rad", &s, "pi/3", NULL);
        num_atan(&s, &(num_t){ .v = { 0, 0, 0 }, .ex = { EX_QS, 0, 0, 1, 1 } }, ANG_DEG);
        expect("atan(0)", &s, "0", NULL);
    }
    /* non-special exact angle falls back to the decimal function */
    {
        num_t fifteen = I(15);
        num_sin(&r, &fifteen, ANG_DEG);
        expect("sin(15deg) decimal", &r, "~", "2.588190451025208E-01");
    }
}

static void test_roots_powers(void)
{
    num_t r, a, b;

    a = I(8);
    b = I(3);
    num_root(&r, &b, &a);
    expect("cbrt(8)", &r, "2", "2.000000000000000E+00");
    a = I(9);
    num_root(&r, &b, &a);
    expect("cbrt(9) decimal", &r, "~", "2.080083823051904E+00");
    a = I(-8);
    num_root(&r, &b, &a);
    expect("cbrt(-8)", &r, "-2", "-2.000000000000000E+00");
    {
        num_t two = I(2), ten = I(10);
        num_pow(&r, &two, &ten);
        expect("2^10", &r, "1024", NULL);
        b = I(-3);
        num_pow(&r, &two, &b);
        expect("2^-3", &r, "1/8", "1.250000000000000E-01");
    }
    {   /* (-8)^(1/3) = -2 via exact rational exponent */
        num_t third, m8 = I(-8), one = I(1), three = I(3);
        num_div(&third, &one, &three);
        num_pow(&r, &m8, &third);
        expect("(-8)^(1/3)", &r, "-2", "-2.000000000000000E+00");
    }
    {   /* 4^(3/2) = 8 ; 2^(1/2) = sqrt(2) */
        num_t e, four = I(4), three = I(3), two = I(2), one = I(1);
        num_div(&e, &three, &two);
        num_pow(&r, &four, &e);
        expect("4^(3/2)", &r, "8", NULL);
        num_div(&e, &one, &two);
        num_pow(&r, &two, &e);
        expect("2^(1/2)", &r, "sqrt(2)", NULL);
    }
    {   /* factorial and combinations are exact integers */
        num_t n = I(10), k = I(3);
        num_fact(&r, &n);
        expect("10!", &r, "3628800", NULL);
        num_ncr(&r, &n, &k);
        expect("10C3", &r, "120", NULL);
        num_npr(&r, &n, &k);
        expect("10P3", &r, "720", NULL);
    }
}

int main(void)
{
    test_rational();
    test_surds();
    test_pi();
    test_trig();
    test_roots_powers();
    printf("%d checks, %d failures\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
