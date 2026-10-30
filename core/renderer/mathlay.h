/*
 * Natural (textbook) layout of a token stream: stacked fractions, radicals with
 * vinculum, raised exponents. One pass measures, a second draws; the cursor
 * position is reported for the editor.
 */
#ifndef TELECO_RENDERER_MATHLAY_H
#define TELECO_RENDERER_MATHLAY_H

#include "renderer/lcd.h"
#include "parser/token.h"

typedef struct box {
    int w;        /* width in pixels                         */
    int asc;      /* rows above the baseline (incl. glyphs)  */
    int desc;     /* rows below the baseline                 */
} box_t;

typedef struct lay_cursor {
    int found;
    int x, y, h;  /* top-left of the cursor bar and its height */
} lay_cursor_t;

/* Size of the whole stream. */
box_t lay_measure(const tok_t *t, int n, int small);
/*
 * Draw with the baseline at yb and left edge at x. If cur >= 0 the cursor
 * position (token boundary index) is stored in *cc (not drawn).
 */
box_t lay_draw(lcd_t *l, const tok_t *t, int n, int x, int yb, int small, int cur, lay_cursor_t *cc);
/* x position (relative to the left edge) of token boundary cur. */
int lay_cursor_x(const tok_t *t, int n, int small, int cur);

#endif
