#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc_script.h"

void script_init(script_t *s)
{
    memset(s, 0, sizeof *s);
    ctx_init(&s->ctx);
    s->exact = 1;
    s->ascii = 1;
}

static int setting(script_t *s, const char *line, char *out, size_t cap)
{
    ctx_t *c = &s->ctx;
    if (line[0] != ':' || (line[1] >= '0' && line[1] <= '9') || line[1] == 0) return 0;
    if (!strcmp(line, ":deg")) c->angle = ANG_DEG;
    else if (!strcmp(line, ":rad")) c->angle = ANG_RAD;
    else if (!strcmp(line, ":grad")) c->angle = ANG_GRAD;
    else if (!strcmp(line, ":norm1")) c->fmt = FMT_NORM1;
    else if (!strcmp(line, ":norm2")) c->fmt = FMT_NORM2;
    else if (!strncmp(line, ":fix ", 5)) { c->fmt = FMT_FIX; c->fmt_digits = (uint8_t)atoi(line + 5); }
    else if (!strncmp(line, ":sci ", 5)) { c->fmt = FMT_SCI; c->fmt_digits = (uint8_t)atoi(line + 5); }
    else if (!strcmp(line, ":eng")) { c->fmt = FMT_ENG; c->fmt_digits = 0; }
    else if (!strcmp(line, ":decimal")) s->exact = 0;
    else if (!strcmp(line, ":exact")) s->exact = 1;
    else if (!strcmp(line, ":raw")) s->raw = 1;
    else if (!strcmp(line, ":noraw")) s->raw = 0;
    else if (!strcmp(line, ":improper")) c->frac_improper = 1;
    else if (!strcmp(line, ":mixed")) c->frac_improper = 0;
    else if (!strcmp(line, ":cmplx on")) c->cplx = 1;
    else if (!strcmp(line, ":cmplx off")) c->cplx = 0;
    else if (!strcmp(line, ":reset")) { int ex = s->exact, as = s->ascii; script_init(s); s->exact = ex; s->ascii = as; }
    else { snprintf(out, cap, "unknown setting: %s", line); return 2; }
    return 1;
}

int script_line(script_t *s, const char *line, char *out, size_t cap)
{
    val_t v;
    calc_err_t e;
    int r;
    while (*line == ' ' || *line == '\t') line++;
    if (!*line || *line == '#') return 0;
    r = setting(s, line, out, cap);
    if (r == 2) return 1;
    if (r == 1) return 0;
    e = calc_eval_text(&s->ctx, line, &v);
    if (e) { snprintf(out, cap, "%s", calc_err_name(e)); return 1; }
    if (s->raw && v.k == V_REAL) dec_to_str(&v.re.v, out, cap);
    else fmt_value_text(&s->ctx, &v, s->exact, s->ascii, out, cap);
    return 1;
}
