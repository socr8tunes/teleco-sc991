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

static const char *const STAT_NAMES[] = { "1var", "lin", "quad", "log", "exp", "ab", "pow", "inv" };

/* ":data x [y [f]]" rows, values separated by commas or spaces; several rows with ';' */
static int data_cmd(script_t *s, const char *arg, char *out, size_t cap)
{
    stat_t *st = &s->ctx.stat;
    while (*arg == ' ') arg++;
    if (!strcmp(arg, "clear")) { stat_init(st, st->type); return 1; }
    if (!strcmp(arg, "list")) {
        size_t pos = 0; int i;
        out[0] = 0;
        for (i = 0; i < st->rows && pos + 64 < cap; i++) {
            char a[40], b[40];
            dec_to_str(&st->x[i], a, sizeof a); dec_to_str(&st->y[i], b, sizeof b);
            pos += (size_t)snprintf(out + pos, cap - pos, "%d: %s %s x%d\n", i + 1, a, b, st->f[i]);
        }
        if (pos == 0) snprintf(out, cap, "(no data)");
        return 2;
    }
    while (*arg) {
        {
            char buf[64]; dec_t v[3]; int nv = 0;
            while (*arg && *arg != ';' && nv < 3) {
                int k = 0;
                while (*arg == ' ' || *arg == ',') arg++;
                if (!*arg || *arg == ';') break;
                while (*arg && *arg != ' ' && *arg != ',' && *arg != ';' && k < 60) buf[k++] = *arg++;
                buf[k] = 0;
                if (dec_from_str(&v[nv], buf) != DEC_OK) { snprintf(out, cap, "bad number: %s", buf); return 2; }
                nv++;
            }
            if (*arg == ';') arg++;
            if (nv == 0) continue;
            {
                int two = st->type != ST_1VAR, freq = 1;
                int64_t fi;
                dec_t zero; dec_zero(&zero);
                if (two) {
                    if (nv < 2) { snprintf(out, cap, "need x and y"); return 2; }
                    if (nv == 3) { if (dec_to_i64(&v[2], &fi)) { snprintf(out, cap, "bad frequency"); return 2; } freq = (int)fi; }
                } else if (nv >= 2) {
                    if (dec_to_i64(&v[1], &fi)) { snprintf(out, cap, "bad frequency"); return 2; } freq = (int)fi;
                }
                {
                    calc_err_t e = stat_add(st, &v[0], two ? &v[1] : &zero, freq);
                    if (e) { snprintf(out, cap, "%s", calc_err_name(e)); return 2; }
                }
            }
        }
    }
    return 1;
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
    else if (!strcmp(line, ":polar")) c->polar = 1;
    else if (!strcmp(line, ":rect")) c->polar = 0;
    else if (!strncmp(line, ":stat ", 6)) {
        int i, ok = 0;
        for (i = 0; i < 8; i++) if (!strcmp(line + 6, STAT_NAMES[i])) { stat_init(&c->stat, i); ok = 1; }
        if (!ok) { snprintf(out, cap, "unknown stat type"); return 2; }
    }
    else if (!strncmp(line, ":data ", 6)) return data_cmd(s, line + 6, out, cap);
    else if (!strcmp(line, ":comp")) c->base = 0;
    else if (!strcmp(line, ":bin")) c->base = 2;
    else if (!strcmp(line, ":oct")) c->base = 8;
    else if (!strcmp(line, ":hex")) c->base = 16;
    else if (!strcmp(line, ":dec")) c->base = 10;
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

int script_aux(const script_t *s, char *out, size_t cap)
{
    val_t v;
    size_t n;
    if (s->ctx.nres > 1) {
        int i;
        const char *names = s->ctx.res_kind == 2 ? "xyz" : "123";
        size_t pos = 0;
        out[0] = 0;
        for (i = 1; i < s->ctx.nres && pos + 8 < cap; i++) {
            pos += (size_t)snprintf(out + pos, cap - pos, "%s%c%s = ", i > 1 ? "\n" : "", s->ctx.res_kind == 2 ? names[i] : 'x', s->ctx.res_kind == 2 ? "" : (i == 1 ? "2" : "3"));
            pos += fmt_value_text(&s->ctx, &s->ctx.res[i], s->exact, s->ascii, out + pos, cap - pos);
        }
        return 1;
    }
    if (!s->ctx.has_aux) return 0;
    val_real(&v, &s->ctx.aux);
    n = (size_t)snprintf(out, cap, s->ctx.has_aux == 1 ? "L-R = " : "(2nd) = ");
    fmt_value_text(&s->ctx, &v, s->exact, s->ascii, out + n, cap - n);
    return 1;
}
