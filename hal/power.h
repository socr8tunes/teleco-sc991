#ifndef TELECO_HAL_POWER_H
#define TELECO_HAL_POWER_H

#include <stdint.h>

typedef enum hal_wake_reason {
    HAL_WAKE_KEY = 0,       /* a key was pressed */
    HAL_WAKE_TIMEOUT,       /* the requested timeout elapsed */
    HAL_WAKE_RESET          /* cold start / power returned after full loss */
} hal_wake_reason;

/*
 * Enter the lowest-power sleep state that keeps RAM retained and return when a
 * key is pressed or `timeout_ms` elapses (0 = no timeout).
 */
hal_wake_reason hal_power_sleep(uint32_t timeout_ms);

/* Power down for auto power-off: retained state is kept, display off. */
void hal_power_off(void);

/* Non-zero when supply voltage is too low to guarantee reliable operation. */
int hal_power_battery_low(void);

#endif /* TELECO_HAL_POWER_H */
