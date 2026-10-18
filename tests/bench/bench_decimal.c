/*
 * Micro-benchmark for the decimal core.
 *
 * PC numbers are only a smoke test: a Cortex-M0+ has no 64-bit divide and no
 * 64x64 multiply, so the real figures must be measured on the STM32U0 board.
 * The same file can be reused there by replacing clock() with a cycle counter.
 */
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "numeric/decimal.h"

enum { POOL = 1024, ITERS = 2000000 };

static uint64_t g_state = 0x9E3779B97F4A7C15ULL;

static uint64_t rnd(void)
{
    g_state ^= g_state << 13;
    g_state ^= g_state >> 7;
    g_state ^= g_state << 17;
    return g_state;
}

static void fill(dec_t *p)
{
    p->m = DEC_MANT_MIN + rnd() % (DEC_MANT_LIMIT - DEC_MANT_MIN);
    p->exp = (int16_t)((int)(rnd() % 61) - 30);
    p->neg = (uint8_t)(rnd() & 1u);
}

static volatile uint64_t g_sink;

static void bench(const char *name, int op, const dec_t *a, const dec_t *b)
{
    dec_t r;
    uint64_t acc = 0;
    clock_t t0, t1;
    int i;

    t0 = clock();
    for (i = 0; i < ITERS; i++) {
        const dec_t *x = &a[i & (POOL - 1)];
        const dec_t *y = &b[(i * 7) & (POOL - 1)];
        switch (op) {
        case 0: dec_add(&r, x, y); break;
        case 1: dec_sub(&r, x, y); break;
        case 2: dec_mul(&r, x, y); break;
        case 3: dec_div(&r, x, y); break;
        default: dec_sqrt(&r, x); break;
        }
        acc += r.m;
    }
    t1 = clock();
    g_sink = acc;
    printf("%-5s %8.1f ns/op\n", name, (double)(t1 - t0) * 1e9 / CLOCKS_PER_SEC / ITERS);
}

int main(void)
{
    static dec_t a[POOL], b[POOL], pos[POOL];
    int i;

    for (i = 0; i < POOL; i++) {
        fill(&a[i]);
        fill(&b[i]);
        pos[i] = a[i];
        pos[i].neg = 0;
    }

    printf("sizeof(dec_t) = %u bytes\n", (unsigned)sizeof(dec_t));
    bench("add", 0, a, b);
    bench("sub", 1, a, b);
    bench("mul", 2, a, b);
    bench("div", 3, a, b);
    bench("sqrt", 4, pos, b);
    return 0;
}
