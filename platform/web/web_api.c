/* WebAssembly entry points: the whole calculator behind a handful of C functions. */
#include "ui/app.h"
#include "ui/keys.h"
#include "parser/tokinfo.h"
#include "calc/errors.h"

static app_t A;
static lcd_t L;
static char text[256];

#define EXPORT __attribute__((visibility("default")))

EXPORT void teleco_init(void) { app_init(&A); app_tick(&A, 0); }
EXPORT void teleco_key(int k) { app_key(&A, k); }
EXPORT void teleco_tick(unsigned ms) { app_tick(&A, ms); }
EXPORT int  teleco_key_count(void) { return K_COUNT; }
EXPORT int  teleco_width(void) { return LCD_W; }
EXPORT int  teleco_height(void) { return LCD_H; }
EXPORT int  teleco_stride(void) { return LCD_STRIDE; }
EXPORT int  teleco_screen(void) { return A.screen; }
EXPORT int  teleco_beep(void) { return A.beep; }
EXPORT int  teleco_power_off(void) { return A.screen == SCR_OFF; }
EXPORT void teleco_set_battery_low(int v) { A.battery_low = (uint8_t)v; }

/* Render and return a pointer to the 1-bit framebuffer (MSB = leftmost pixel). */
EXPORT const unsigned char *teleco_render(void)
{
    app_render(&A, &L);
    return L.px;
}

/* Text input buffer for loading an expression (tests / paste). */
EXPORT char *teleco_text_buf(void) { return text; }
EXPORT int teleco_text_size(void) { return (int)sizeof text; }
EXPORT int teleco_load_expr(void) { text[sizeof text - 1] = 0; return ui_set_expr_text(&A, text); }

/* Result currently shown, as ASCII (value or error name). */
EXPORT const char *teleco_result(void)
{
    text[0] = 0;
    if (!A.post_eval) return text;
    if (!A.have_res) {
        const char *e = calc_err_name((calc_err_t)A.err);
        int i = 0;
        while (e[i] && i < (int)sizeof text - 1) { text[i] = e[i]; i++; }
        text[i] = 0;
        return text;
    }
    {
        val_t v = A.res;
        if (A.ctx.nres > 1) v = A.ctx.res[A.view_idx < A.ctx.nres ? A.view_idx : 0];
        else if (A.ctx.has_aux && A.view_idx == 1) val_real(&v, &A.ctx.aux);
        (void)fmt_value_text(&A.ctx, &v, A.exact, 1, text, sizeof text);
    }
    return text;
}

EXPORT int teleco_contrast(void) { return A.contrast; }
