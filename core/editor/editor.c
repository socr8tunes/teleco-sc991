#include "editor/editor.h"
#include "parser/tokinfo.h"
#include <string.h>

void ed_clear(editor_t *e) { e->n = 0; e->cur = 0; }

void ed_set(editor_t *e, const tok_t *t, int n)
{
    if (n > ED_CAP) n = ED_CAP;
    memcpy(e->t, t, (size_t)n * sizeof(tok_t));
    e->n = n;
    e->cur = n;
}

static int make_room(editor_t *e, int k)
{
    if (e->n + k > ED_CAP) return -1;
    memmove(&e->t[e->cur + k], &e->t[e->cur], (size_t)(e->n - e->cur) * sizeof(tok_t));
    e->n += k;
    return 0;
}

int ed_insert(editor_t *e, tok_t k)
{
    if (make_room(e, 1)) return -1;
    e->t[e->cur++] = k;
    return 0;
}

int ed_nest_depth(const editor_t *e)
{
    int i, d = 0;
    for (i = 0; i < e->cur && i < e->n; i++) {
        if (tok_struct_end(e->t[i].t)) d++;
        else if (tok_is_struct_end(e->t[i].t)) d--;
    }
    return d < 0 ? 0 : d;
}

int ed_insert_struct(editor_t *e, uint8_t b)
{
    int seps, first = e->cur, k, j;
    uint8_t sep1 = 0, sep2 = 0, end = tok_struct_end(b);
    if (!end || ed_nest_depth(e) >= ED_MAX_NEST) return -1;
    switch (b) {
    case T_FRAC: sep1 = T_FRACSEP; break;
    case T_MFRAC: sep1 = T_MSEP1; sep2 = T_MSEP2; break;
    case T_ROOT: sep1 = T_ROOTSEP; break;
    default: break;
    }
    seps = (sep1 != 0) + (sep2 != 0);
    if (b == T_FRAC) {
        /* a number typed just before becomes the numerator */
        while (first > 0 && tok_is_digit(e->t[first - 1].t)) first--;
    }
    k = e->cur - first;
    if (e->n + 2 + seps > ED_CAP) return -1;
    /* shift tail right by 2+seps, then place tokens */
    memmove(&e->t[e->cur + 2 + seps], &e->t[e->cur], (size_t)(e->n - e->cur) * sizeof(tok_t));
    e->n += 2 + seps;
    /* digits move one position right to make room for the begin token */
    for (j = k - 1; j >= 0; j--) e->t[first + 1 + j] = e->t[first + j];
    e->t[first] = mk_tok(b, 0);
    j = first + 1 + k;
    if (sep1) e->t[j++] = mk_tok(sep1, 0);
    if (sep2) e->t[j++] = mk_tok(sep2, 0);
    e->t[j++] = mk_tok(end, 0);
    /* cursor: after the numerator for a fraction made from digits (denominator), else first slot */
    if (b == T_FRAC && k > 0) e->cur = first + 1 + k + 1;
    else e->cur = first + 1;
    return 0;
}

void ed_left(editor_t *e) { if (e->cur > 0) e->cur--; }
void ed_right(editor_t *e) { if (e->cur < e->n) e->cur++; }
void ed_home(editor_t *e) { e->cur = 0; }
void ed_end(editor_t *e) { e->cur = e->n; }

static int struct_empty(const editor_t *e, int b, int end)
{
    int i;
    for (i = b + 1; i < end; i++)
        if (!tok_is_struct_sep(e->t[i].t)) return 0;
    return 1;
}

void ed_delete(editor_t *e)
{
    int c = e->cur;
    uint8_t k;
    if (c == 0) return;
    k = e->t[c - 1].t;
    if (tok_is_struct_end(k) || tok_is_struct_sep(k)) { e->cur--; return; }
    if (tok_struct_end(k)) {
        int m = tok_struct_match(e->t, e->n, c - 1);
        if (m < e->n && struct_empty(e, c - 1, m)) {
            int cnt = m - (c - 1) + 1;
            memmove(&e->t[c - 1], &e->t[c - 1 + cnt], (size_t)(e->n - (c - 1 + cnt)) * sizeof(tok_t));
            e->n -= cnt;
            e->cur = c - 1;
        } else {
            e->cur--;
        }
        return;
    }
    memmove(&e->t[c - 1], &e->t[c], (size_t)(e->n - c) * sizeof(tok_t));
    e->n--;
    e->cur--;
}

/* innermost fraction-like structure containing the cursor; returns its begin index or -1 */
static int enclosing(const editor_t *e, int *slot)
{
    int stack[ED_MAX_NEST + 2], sl[ED_MAX_NEST + 2], sp = 0, i;
    for (i = 0; i < e->cur && i < e->n; i++) {
        uint8_t k = e->t[i].t;
        if (tok_struct_end(k)) {
            if (sp < ED_MAX_NEST + 2) { stack[sp] = i; sl[sp] = 0; sp++; }
        } else if (tok_is_struct_end(k)) {
            if (sp > 0) sp--;
        } else if (tok_is_struct_sep(k)) {
            if (sp > 0) sl[sp - 1]++;
        }
    }
    if (sp == 0) return -1;
    *slot = sl[sp - 1];
    return stack[sp - 1];
}

int ed_vertical(editor_t *e, int dir)
{
    int slot = 0, b = enclosing(e, &slot), end, i;
    uint8_t bt;
    if (b < 0) return 0;
    bt = e->t[b].t;
    end = tok_struct_match(e->t, e->n, b);
    if (bt == T_POW) {
        if (dir > 0 && end < e->n) { e->cur = end + 1; return 1; }
        return 0;
    }
    if (bt != T_FRAC && bt != T_MFRAC) return 0;
    {
        int want = slot + dir;
        int lo = bt == T_FRAC ? 0 : 1, hi = bt == T_FRAC ? 1 : 2;
        if (want < lo || want > hi) return 0;
        /* position at the end of slot `want` */
        {
            int slot_i = 0;
            for (i = b + 1; i <= end; i++) {
                if (i < end && tok_struct_end(e->t[i].t)) { i = tok_struct_match(e->t, e->n, i); continue; }
                if (i == end || tok_is_struct_sep(e->t[i].t)) {
                    if (slot_i == want) { e->cur = i; return 1; }
                    slot_i++;
                }
            }
        }
    }
    return 0;
}

int ed_build(const editor_t *e, tok_t *out, int cap)
{
    return tok_autoclose(e->t, e->n, out, cap);
}
