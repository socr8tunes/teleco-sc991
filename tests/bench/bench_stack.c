/*
 * Worst-case stack depth of calculations, measured by stack painting on the host (64-bit; frames are
 * typically no smaller than on a 32-bit Cortex-M, so this is a reasonable upper estimate, but it is NOT a
 * hardware measurement). Prints the high-water mark and fails if it exceeds the budget given as argv[1] (bytes).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc/eval.h"
#include "calc/format.h"

#define PAINT 0xA5u
#define REGION (256u * 1024u)

static uintptr_t region_lo;

__attribute__((noinline)) static void paint(void)
{
    volatile unsigned char buf[REGION];
    size_t i;
    for (i = 0; i < REGION; i++) buf[i] = PAINT;
    region_lo = (uintptr_t)(const volatile void *)buf;
}

__attribute__((noinline)) static size_t used(void)
{
    size_t i = 0;
    while (i < REGION && ((const volatile unsigned char *)region_lo)[i] == PAINT) i++;
    return REGION - i;
}

int main(int argc, char **argv)
{
    static const char *const E[] = {
        "2+3*4", "69!", "sum(1/X^2,1,1000)", "ddx(sin(X),1)", "integral(e^(-X^2),0,3)", "integral(1/X,1,100)",
        "solve(cos(X)=X,1)", "solve(X^3-2X-5=0,2)", "simul3(1,2,3,4,5,6,7,8,10,1,2,3)",
        "[1,2,3;4,5,6;7,8,10]^-1", "det([1,2,3,4;5,6,7,9;1,0,1,2;3,1,4,1])", "cubic(1,-6,11,-6)",
        "(1+2i)^10", "((((((((1+2)*3)+4)*5)+6)*7)+8)*9)", "integral(integral(X*Y,0,1),0,1)",
    };
    static ctx_t c;
    size_t i, worst = 0, base;
    long budget = argc > 1 ? atol(argv[1]) : 0;
    ctx_init(&c);
    c.cplx = 1;
    paint(); base = used();
    for (i = 0; i < sizeof E / sizeof E[0]; i++) {
        val_t v;
        char out[96];
        paint();
        if (calc_eval_text(&c, E[i], &v) == CE_OK) (void)fmt_value_text(&c, &v, 1, 1, out, sizeof out);
        {
            size_t u = used();
            printf("%-44s %6zu bytes\n", E[i], u);
            if (u > worst) worst = u;
        }
    }
    printf("worst stack high-water: %zu bytes (empty baseline %zu)\n", worst, base);
    if (budget && (long)worst > budget) { printf("FAIL: over budget %ld\n", budget); return 1; }
    return 0;
}
