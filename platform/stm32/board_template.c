/*
 * Board HAL TEMPLATE for a Cortex-M target. Every function is a stub that satisfies the HAL header contract
 * (so the firmware links) but drives NO hardware. Replace each TODO with the real driver once the
 * board, LCD controller and key matrix are chosen. Nothing here has been run on a device.
 * Compile check: tools/check_template.sh
 */
#include <stddef.h>
#include <stdint.h>
#include "hal/display.h"
#include "hal/keypad.h"
#include "hal/power.h"
#include "hal/storage.h"
#include "hal/timebase.h"

#define LCD_W 192u
#define LCD_H 64u

static volatile uint32_t g_millis;     /* TODO: incremented by SysTick/LPTIM interrupt; must keep counting in sleep */

void board_init(void)
{
    /* TODO: clocks (MSI low-speed for battery), GPIO, SPI to LCD, key matrix GPIO + EXTI, RTC/LPTIM tick, BOR/PVD. */
}

uint32_t hal_millis(void) { return g_millis; }

static const hal_display_info g_info = { LCD_W, LCD_H };
const hal_display_info *hal_display_get_info(void) { return &g_info; }
void hal_display_present(const uint8_t *fb) { (void)fb; /* TODO: page-by-page SPI transfer to the controller */ }
void hal_display_set_contrast(uint8_t level) { (void)level; /* TODO: controller contrast / bias command */ }
void hal_display_power(int on) { (void)on; /* TODO: display on/off + bias supply */ }

int hal_keypad_scan(hal_key_t *keys, int max)
{
    (void)keys; (void)max;
    /* TODO: drive rows, read columns, debounce (~10 ms), map to the key index in core/ui/keys.h order */
    return 0;
}
void hal_keypad_arm_wake(void) { /* TODO: all rows low, column EXTI enabled */ }

hal_wake_reason hal_power_sleep(uint32_t timeout_ms)
{
    (void)timeout_ms;
    /* TODO: STOP mode with RAM retention; wake on key EXTI or LPTIM/RTC timeout */
    return HAL_WAKE_TIMEOUT;
}
void hal_power_off(void) { /* TODO: display off, deepest retained state (STOP2 / standby with backup SRAM) */ }
int hal_power_battery_low(void) { return 0; /* TODO: PVD or ADC measurement of the supply */ }

int hal_storage_begin(size_t total) { (void)total; return -1; /* TODO: erase target flash page(s) / backup SRAM */ }
int hal_storage_write(const void *data, size_t len) { (void)data; (void)len; return -1; }
int hal_storage_commit(void) { return -1; /* TODO: switch the valid-slot marker last (two-slot wear levelling) */ }
int hal_storage_open(size_t *len) { (void)len; return -1; }
int hal_storage_read(void *buf, size_t len) { (void)buf; (void)len; return -1; }
void hal_storage_clear(void) { }
