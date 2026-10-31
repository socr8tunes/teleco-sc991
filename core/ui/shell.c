#include "ui/shell.h"
#include "ui/persist.h"
#include "hal/display.h"
#include "hal/power.h"
#include "hal/timebase.h"
#include "hal/storage.h"
#include <string.h>

int shell_init(shell_t *s)
{
    int restored;
    memset(s, 0, sizeof *s);
    app_init(&s->app);
    restored = persist_load(&s->app) == 0;
    app_tick(&s->app, hal_millis());
    s->app.logo_ms = hal_millis();
    s->app.last_key_ms = hal_millis();
    hal_display_power(1);
    hal_display_set_contrast(s->app.contrast);
    s->panel_on = 1;
    s->dirty = 1;
    s->last_contrast = s->app.contrast;
    return restored;
}

static int is_held(const shell_t *s, hal_key_t k)
{
    int i;
    for (i = 0; i < s->nheld; i++) if (s->held[i] == k) return 1;
    return 0;
}

void shell_step(shell_t *s)
{
    hal_key_t keys[HAL_KEYPAD_MAX_ROLLOVER];
    int n, i, pressed = 0;
    uint32_t now = hal_millis();

    n = hal_keypad_scan(keys, HAL_KEYPAD_MAX_ROLLOVER);
    if (n < 0) n = 0;
    if (n > HAL_KEYPAD_MAX_ROLLOVER) n = HAL_KEYPAD_MAX_ROLLOVER;
    s->app.now_ms = now;
    for (i = 0; i < n; i++) {
        if (keys[i] < K_COUNT && !is_held(s, keys[i])) {
            app_key(&s->app, keys[i]);
            pressed = 1;
        }
    }
    s->nheld = (uint8_t)n;
    for (i = 0; i < n; i++) s->held[i] = keys[i];

    app_tick(&s->app, now);
    {
        uint8_t low = (uint8_t)(hal_power_battery_low() != 0);
        if (low != s->app.battery_low) { s->app.battery_low = low; s->dirty = 1; }
    }
    if (pressed || s->app.screen != s->last_screen || s->app.blink != s->last_blink) s->dirty = 1;

    if (s->app.screen == SCR_OFF) {
        if (s->panel_on) {
            (void)persist_save(&s->app);        /* keep state across a full power loss */
            hal_display_power(0);
            s->panel_on = 0;
        }
    } else if (!s->panel_on) {
        hal_display_power(1);
        hal_display_set_contrast(s->app.contrast);
        s->panel_on = 1;
        s->dirty = 1;
    }
    if (s->app.contrast != s->last_contrast) {
        hal_display_set_contrast(s->app.contrast);
        s->last_contrast = s->app.contrast;
    }
    if (s->app.beep) s->app.beep = 0;           /* no buzzer on this product; hook point for feedback */

    if (s->dirty && s->panel_on) {
        app_render(&s->app, &s->lcd);
        hal_display_present(s->lcd.px);
        s->dirty = 0;
    }
    s->last_screen = s->app.screen;
    s->last_blink = s->app.blink;

    if (n == 0 && !s->dirty) {
        hal_keypad_arm_wake();
        (void)hal_power_sleep(s->panel_on ? 250u : 0u);   /* 250 ms: cursor blink; off: wait for a key */
    }
}
