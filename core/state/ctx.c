#include <string.h>
#include "state/ctx.h"

void ctx_init(ctx_t *c)
{
    int i;
    memset(c, 0, sizeof *c);
    c->angle = ANG_DEG;
    c->fmt = FMT_NORM1;
    for (i = 0; i < N_VARS; i++) val_zero(&c->var[i]);
    val_zero(&c->ans);
    val_zero(&c->preans);
    c->rng = 0x2545F491u;
}
