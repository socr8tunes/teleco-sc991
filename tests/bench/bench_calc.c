#define _POSIX_C_SOURCE 199309L
/* Host timing of complete calculations (relative cost guide; NOT a hardware measurement). */
#include <stdio.h>
#include <time.h>
#include "calc/eval.h"
#include "calc/format.h"

static double now_us(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e6 + (double)t.tv_nsec / 1e3;
}

int main(void)
{
    static const char *const E[] = {
        "2+3*4", "sin(30)", "sqrt(2)+pi", "ln(7)^2", "69!", "tan(1.234)", "asinh(2.5)", "7^(1/3)",
        "sum(1/X^2,1,1000)", "ddx(sin(X),1)", "integral(sin(X),0,pi)", "integral(e^(-X^2),0,3)",
        "integral(1/X,1,100)", "solve(X^3-2X-5=0,2)", "solve(cos(X)=X,1)",
        "simul3(1,2,3,4,5,6,7,8,10,1,2,3)", "[1,2,3;4,5,6;7,8,10]^-1", "det([1,2,3,4;5,6,7,9;1,0,1,2;3,1,4,1])",
        "cubic(1,-6,11,-6)", "normp(1.5)", "(1+2i)^10",
    };
    static ctx_t c;
    size_t i;
    ctx_init(&c);
    c.cplx = 1;
    printf("%-44s %12s\n", "expression", "host us/calc");
    for (i = 0; i < sizeof E / sizeof E[0]; i++) {
        val_t v;
        char out[96];
        int reps = 0;
        double t0 = now_us(), t1;
        do {
            (void)calc_eval_text(&c, E[i], &v);
            reps++;
            t1 = now_us();
        } while (t1 - t0 < 100000.0 && reps < 100000);
        fmt_value_text(&c, &v, 1, 1, out, sizeof out);
        printf("%-44s %12.1f   -> %s\n", E[i], (t1 - t0) / reps, out);
    }
    return 0;
}
