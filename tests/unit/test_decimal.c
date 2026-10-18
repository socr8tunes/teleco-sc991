/* Known-value unit tests for core/numeric. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "numeric/decimal.h"

static int g_checks;
static int g_failures;

static void fail_report(const char *what, const char *got, const char *want)
{
    g_failures++;
    printf("FAIL: %s\n      got:  %s\n      want: %s\n", what, got, want);
}

static void check_true(const char *what, int cond)
{
    g_checks++;
    if (!cond) {
        fail_report(what, "false", "true");
    }
}

/* Parse operands, run op, compare canonical string and status. */
static void check_op(const char *op, const char *sa, const char *sb,
                     const char *want_str, dec_status want_st)
{
    dec_t a, b, r;
    char buf[DEC_STR_MAX];
    char what[160];
    dec_status st = DEC_OK;

    g_checks++;
    snprintf(what, sizeof what, "%s(%s, %s)", op, sa, sb ? sb : "-");

    if (dec_from_str(&a, sa) != DEC_OK || (sb && dec_from_str(&b, sb) != DEC_OK)) {
        /* Operands that intentionally parse to zero (underflow) are fine; others are test bugs. */
        if (!dec_is_zero(&a)) {
            fail_report(what, "operand did not parse", "OK");
            return;
        }
    }

    if (strcmp(op, "add") == 0) {
        st = dec_add(&r, &a, &b);
    } else if (strcmp(op, "sub") == 0) {
        st = dec_sub(&r, &a, &b);
    } else if (strcmp(op, "mul") == 0) {
        st = dec_mul(&r, &a, &b);
    } else if (strcmp(op, "div") == 0) {
        st = dec_div(&r, &a, &b);
    } else if (strcmp(op, "sqrt") == 0) {
        st = dec_sqrt(&r, &a);
    } else {
        fail_report(what, "unknown op", "known op");
        return;
    }

    dec_to_str(&r, buf, sizeof buf);
    if (strcmp(buf, want_str) != 0) {
        fail_report(what, buf, want_str);
    } else if (st != want_st) {
        fail_report(what, dec_status_name(st), dec_status_name(want_st));
    } else if (!dec_is_valid(&r)) {
        fail_report(what, "non-canonical result", "canonical");
    }
}

static void check_parse(const char *s, const char *want_str, dec_status want_st)
{
    dec_t r;
    char buf[DEC_STR_MAX];
    char what[160];
    dec_status st;

    g_checks++;
    snprintf(what, sizeof what, "parse(\"%s\")", s);
    st = dec_from_str(&r, s);
    dec_to_str(&r, buf, sizeof buf);
    if (strcmp(buf, want_str) != 0) {
        fail_report(what, buf, want_str);
    } else if (st != want_st) {
        fail_report(what, dec_status_name(st), dec_status_name(want_st));
    }
}

static void test_add_sub(void)
{
    /* The classic: decimal-correct. */
    check_op("add", "0.1", "0.2", "3.000000000000000E-01", DEC_OK);
    check_op("add", "1", "2", "3.000000000000000E+00", DEC_OK);
    check_op("add", "-2", "3", "1.000000000000000E+00", DEC_OK);
    check_op("add", "0", "0", "0", DEC_OK);
    check_op("add", "0", "-7.5", "-7.500000000000000E+00", DEC_OK);
    check_op("sub", "1.5", "1.5", "0", DEC_OK);
    check_op("sub", "10", "0.1", "9.900000000000000E+00", DEC_OK);
    check_op("sub", "0", "4", "-4.000000000000000E+00", DEC_OK);

    /* Ties go to even. */
    check_op("add", "9999999999999999", "0.5", "1.000000000000000E+16", DEC_OK);
    check_op("add", "9999999999999998", "0.5", "9.999999999999998E+15", DEC_OK);

    /* Operand far below half an ulp must not change the result. */
    check_op("add", "1", "1E-17", "1.000000000000000E+00", DEC_OK);
    check_op("add", "1", "1E-90", "1.000000000000000E+00", DEC_OK);
    /* 1 - 1e-17 = 0.99999999999999999 rounds back up to 1. */
    check_op("sub", "1", "1E-17", "1.000000000000000E+00", DEC_OK);
    /* 1 - 1e-16 = 0.9999999999999999 is exactly representable. */
    check_op("sub", "1", "1E-16", "9.999999999999999E-01", DEC_OK);

    /* Range limits. */
    check_op("add", "5E99", "5E99", "0", DEC_OVERFLOW);
    check_op("add", "9.999999999999999E99", "0", "9.999999999999999E+99", DEC_OK);
    check_op("sub", "1.5E-99", "1E-99", "0", DEC_UNDERFLOW);
}

static void test_mul(void)
{
    check_op("mul", "1.5", "2", "3.000000000000000E+00", DEC_OK);
    check_op("mul", "3", "0.1", "3.000000000000000E-01", DEC_OK);
    check_op("mul", "-4", "2.5", "-1.000000000000000E+01", DEC_OK);
    check_op("mul", "0", "123", "0", DEC_OK);

    /* 17-digit products ending in exactly 5: ties to even. */
    check_op("mul", "5000000000000001", "5", "2.500000000000000E+16", DEC_OK);
    check_op("mul", "5000000000000003", "5", "2.500000000000002E+16", DEC_OK);

    check_op("mul", "1E49", "1E50", "1.000000000000000E+99", DEC_OK);
    check_op("mul", "1E50", "1E50", "0", DEC_OVERFLOW);
    check_op("mul", "1E-50", "1E-50", "0", DEC_UNDERFLOW);
}

static void test_div(void)
{
    check_op("div", "1", "3", "3.333333333333333E-01", DEC_OK);
    check_op("div", "2", "3", "6.666666666666667E-01", DEC_OK);
    check_op("div", "1", "8", "1.250000000000000E-01", DEC_OK);
    check_op("div", "7", "7", "1.000000000000000E+00", DEC_OK);
    check_op("div", "-9", "3", "-3.000000000000000E+00", DEC_OK);
    check_op("div", "0", "5", "0", DEC_OK);

    check_op("div", "9999999999999999", "2", "5.000000000000000E+15", DEC_OK); /* tie to even */

    check_op("div", "1", "0", "0", DEC_DIV_BY_ZERO);
    check_op("div", "0", "0", "0", DEC_DOMAIN);
    check_op("div", "1E99", "1E-99", "0", DEC_OVERFLOW);
    check_op("div", "1E-99", "10", "0", DEC_UNDERFLOW);
}

static void test_sqrt(void)
{
    check_op("sqrt", "2", NULL, "1.414213562373095E+00", DEC_OK);
    check_op("sqrt", "9", NULL, "3.000000000000000E+00", DEC_OK);
    check_op("sqrt", "0", NULL, "0", DEC_OK);
    check_op("sqrt", "0.25", NULL, "5.000000000000000E-01", DEC_OK);
    check_op("sqrt", "1E98", NULL, "1.000000000000000E+49", DEC_OK);
    check_op("sqrt", "-4", NULL, "0", DEC_DOMAIN);
}

static void test_parse(void)
{
    check_parse("123.45", "1.234500000000000E+02", DEC_OK);
    check_parse("-0.000123", "-1.230000000000000E-04", DEC_OK);
    check_parse(".5", "5.000000000000000E-01", DEC_OK);
    check_parse("5.", "5.000000000000000E+00", DEC_OK);
    check_parse("+1e5", "1.000000000000000E+05", DEC_OK);
    check_parse("1.5E-3", "1.500000000000000E-03", DEC_OK);
    check_parse("-0", "0", DEC_OK);
    check_parse("0.000", "0", DEC_OK);

    /* More than 16 digits: correct rounding, including ties and sticky tails. */
    check_parse("1.2345678901234565", "1.234567890123456E+00", DEC_OK);
    check_parse("1.2345678901234575", "1.234567890123458E+00", DEC_OK);
    check_parse("1.23456789012345650000000000000000000001", "1.234567890123457E+00", DEC_OK);
    check_parse("12345678901234567890123456789012345678901234567890E-40",
                "1.234567890123457E+09", DEC_OK);

    check_parse("1E100", "0", DEC_OVERFLOW);
    check_parse("1E-100", "0", DEC_UNDERFLOW);
    check_parse("9.9999999999999995E99", "0", DEC_OVERFLOW); /* rounds up to 1E100 */

    check_parse("", "0", DEC_PARSE);
    check_parse("abc", "0", DEC_PARSE);
    check_parse("1e", "0", DEC_PARSE);
    check_parse("1.2.3", "0", DEC_PARSE);
    check_parse("-", "0", DEC_PARSE);
    check_parse(".", "0", DEC_PARSE);
    check_parse("1 2", "0", DEC_PARSE);
}

static void test_misc(void)
{
    dec_t a, b, r;
    char buf[DEC_STR_MAX];

    /* from_i64, including the most negative value. */
    check_true("from_i64(0)", dec_from_i64(&r, 0) == DEC_OK && dec_is_zero(&r));
    dec_from_i64(&r, 42);
    dec_to_str(&r, buf, sizeof buf);
    check_true("from_i64(42)", strcmp(buf, "4.200000000000000E+01") == 0);
    dec_from_i64(&r, INT64_MIN);
    dec_to_str(&r, buf, sizeof buf);
    check_true("from_i64(INT64_MIN)", strcmp(buf, "-9.223372036854776E+18") == 0);

    /* Aliasing: result may overwrite either operand. */
    dec_from_str(&a, "3");
    dec_from_str(&b, "4");
    dec_add(&a, &a, &b);
    dec_to_str(&a, buf, sizeof buf);
    check_true("alias r==a", strcmp(buf, "7.000000000000000E+00") == 0);
    dec_mul(&a, &a, &a);
    dec_to_str(&a, buf, sizeof buf);
    check_true("alias r==a==b", strcmp(buf, "4.900000000000000E+01") == 0);
    dec_sqrt(&a, &a);
    dec_to_str(&a, buf, sizeof buf);
    check_true("alias sqrt", strcmp(buf, "7.000000000000000E+00") == 0);

    /* Comparison and sign helpers. */
    dec_from_str(&a, "-2");
    dec_from_str(&b, "1");
    check_true("cmp -2 < 1", dec_cmp(&a, &b) < 0);
    check_true("cmp 1 > -2", dec_cmp(&b, &a) > 0);
    check_true("cmp 1 == 1", dec_cmp(&b, &b) == 0);
    dec_neg(&r, &a);
    check_true("neg", dec_cmp(&r, &b) > 0 && r.neg == 0);
    dec_zero(&r);
    dec_neg(&r, &r);
    check_true("neg(0) stays canonical", dec_is_valid(&r));
    dec_abs(&r, &a);
    check_true("abs", r.neg == 0 && r.m == a.m);

    /* dec_to_str refuses a buffer that is too small. */
    {
        char small[8];
        check_true("to_str small buffer", dec_to_str(&b, small, sizeof small) == 0);
    }
}

int main(void)
{
    test_add_sub();
    test_mul();
    test_div();
    test_sqrt();
    test_parse();
    test_misc();

    printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
