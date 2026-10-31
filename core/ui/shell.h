/*
 * Main loop body shared by every platform: reads keys through the HAL, drives the
 * app, renders when something changed and sleeps when idle.
 */
#ifndef TELECO_UI_SHELL_H
#define TELECO_UI_SHELL_H

#include "ui/app.h"
#include "hal/keypad.h"

typedef struct shell {
    app_t   app;
    lcd_t   lcd;
    hal_key_t held[HAL_KEYPAD_MAX_ROLLOVER];
    uint8_t nheld;
    uint8_t dirty;
    uint8_t panel_on;
    uint8_t last_screen, last_blink, last_contrast;
    uint8_t shown_battery_low;
    uint32_t power_off_ms;
} shell_t;

/* Initialise, restoring retained state if valid. Returns 1 if state was restored. */
int  shell_init(shell_t *s);
/* One main-loop iteration; sleeps via the HAL when nothing needs doing. */
void shell_step(shell_t *s);

#endif
