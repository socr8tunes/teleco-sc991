/*
 * Natural-display expression editor: a flat token array with a cursor at a
 * token boundary. Structures (fractions, roots, powers) are inserted whole with
 * empty slots and the cursor moves between slots.
 */
#ifndef TELECO_EDITOR_EDITOR_H
#define TELECO_EDITOR_EDITOR_H

#include "parser/token.h"

#define ED_CAP      256
#define ED_MAX_NEST 8

typedef struct editor {
    tok_t t[ED_CAP];
    int n;
    int cur;
} editor_t;

void ed_clear(editor_t *e);
void ed_set(editor_t *e, const tok_t *t, int n);     /* replace content, cursor at end */
int  ed_insert(editor_t *e, tok_t k);                /* 0 ok, -1 buffer full */
/* Insert a natural structure (T_FRAC, T_MFRAC, T_SQRT, T_ROOT, T_POW) with empty slots. */
int  ed_insert_struct(editor_t *e, uint8_t begin);
void ed_left(editor_t *e);
void ed_right(editor_t *e);
void ed_home(editor_t *e);
void ed_end(editor_t *e);
void ed_delete(editor_t *e);                         /* DEL */
/* Move between the slots of the enclosing fraction (dir -1 = up, +1 = down). Returns 1 if moved. */
int  ed_vertical(editor_t *e, int dir);
int  ed_nest_depth(const editor_t *e);               /* structure nesting at the cursor */
/* Tokens ready for the compiler (parentheses closed). Returns count or -1. */
int  ed_build(const editor_t *e, tok_t *out, int cap);

#endif
