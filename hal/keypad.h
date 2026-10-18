#ifndef TELECO_HAL_KEYPAD_H
#define TELECO_HAL_KEYPAD_H

#include <stdint.h>

/* Logical key index assigned by the platform's keymap (0 .. n-1). */
typedef uint8_t hal_key_t;

#define HAL_KEY_NONE            0xFFu
#define HAL_KEYPAD_MAX_ROLLOVER 2     /* product requirement: 2-key rollover */

/*
 * Scan the matrix once, debounced. Writes up to `max` pressed keys into
 * keys[] and returns how many were found (0 if none).
 */
int hal_keypad_scan(hal_key_t *keys, int max);

/*
 * Prepare the keypad to wake the MCU from sleep on any key press
 * (drive all rows, enable column interrupts). Call before hal_power_sleep().
 */
void hal_keypad_arm_wake(void);

#endif /* TELECO_HAL_KEYPAD_H */
