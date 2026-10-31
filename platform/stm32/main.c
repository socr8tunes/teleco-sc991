/*
 * MCU entry point (Cortex-M). Everything hardware specific lives behind the hal headers;
 * this file only owns the main loop. The HAL implementation for the chosen board
 * (display controller, key matrix, low-power modes, retained storage) is part of
 * the bring-up work - see docs/hardware-readiness.md. NOT yet run on hardware.
 */
#include "ui/shell.h"

static shell_t shell;     /* static: ~18 KB, must not live on the stack */

void board_init(void);    /* clocks, GPIO, SPI, RTC/timer for hal_millis (provided by the board HAL) */

int main(void)
{
    board_init();
    (void)shell_init(&shell);
    for (;;) shell_step(&shell);
}
