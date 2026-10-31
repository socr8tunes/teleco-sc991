#include "hal_pc.h"
#include "hal/display.h"
#include "hal/keypad.h"
#include "hal/power.h"
#include "hal/storage.h"
#include "hal/timebase.h"
#include "renderer/lcd.h"
#include <string.h>

#define STORE_MAX 40000

static uint32_t now;
static uint8_t held[8];
static int nheld;
static uint8_t frame[LCD_STRIDE * LCD_H];
static int presents, disp_on = 1, sleeps, batt_low;
static uint8_t store[STORE_MAX], pending[STORE_MAX];
static size_t store_len, pend_len, pend_pos, read_pos;
static int store_valid, pend_ok, fail_commit;

void hal_pc_reset(void)
{
    now = 0; nheld = 0; presents = 0; disp_on = 1; sleeps = 0; batt_low = 0;
    memset(frame, 0, sizeof frame);
}
void hal_pc_set_keys(const uint8_t *k, int n)
{
    int i;
    nheld = n > 8 ? 8 : n;
    for (i = 0; i < nheld; i++) held[i] = k[i];
}
void hal_pc_advance(uint32_t ms) { now += ms; }
const uint8_t *hal_pc_framebuffer(void) { return frame; }
int hal_pc_present_count(void) { return presents; }
int hal_pc_display_on(void) { return disp_on; }
int hal_pc_sleep_count(void) { return sleeps; }
void hal_pc_set_battery_low(int v) { batt_low = v; }
void hal_pc_storage_corrupt(size_t off) { if (off < store_len) store[off] ^= 0x5A; }
size_t hal_pc_storage_size(void) { return store_valid ? store_len : 0; }
void hal_pc_storage_wipe(void) { store_valid = 0; store_len = 0; }
void hal_pc_storage_fail_next_commit(void) { fail_commit = 1; }

/* ---- HAL ---- */
uint32_t hal_millis(void) { return now; }

static const hal_display_info INFO = {LCD_W, LCD_H};
const hal_display_info *hal_display_get_info(void) { return &INFO; }
void hal_display_present(const uint8_t *fb) { memcpy(frame, fb, sizeof frame); presents++; }
void hal_display_set_contrast(uint8_t level) { (void)level; }
void hal_display_power(int on) { disp_on = on; }

int hal_keypad_scan(hal_key_t *keys, int max)
{
    int i, n = nheld < max ? nheld : max;
    for (i = 0; i < n; i++) keys[i] = held[i];
    return n;
}
void hal_keypad_arm_wake(void) {}

hal_wake_reason hal_power_sleep(uint32_t timeout_ms)
{
    sleeps++;
    now += timeout_ms ? timeout_ms : 1000u;   /* fake clock jumps to the wake-up */
    return timeout_ms ? HAL_WAKE_TIMEOUT : HAL_WAKE_KEY;
}
void hal_power_off(void) { disp_on = 0; }
int hal_power_battery_low(void) { return batt_low; }

int hal_storage_begin(size_t total)
{
    if (total > STORE_MAX) return -1;
    pend_len = total; pend_pos = 0; pend_ok = 1;
    return 0;
}
int hal_storage_write(const void *d, size_t n)
{
    if (!pend_ok || pend_pos + n > pend_len) { pend_ok = 0; return -1; }
    memcpy(pending + pend_pos, d, n);
    pend_pos += n;
    return 0;
}
int hal_storage_commit(void)
{
    if (fail_commit) { fail_commit = 0; pend_ok = 0; return -1; }   /* previous blob stays valid */
    if (!pend_ok || pend_pos != pend_len) return -1;
    memcpy(store, pending, pend_len);
    store_len = pend_len; store_valid = 1;
    return 0;
}
int hal_storage_open(size_t *len)
{
    if (!store_valid) return -1;
    *len = store_len; read_pos = 0;
    return 0;
}
int hal_storage_read(void *buf, size_t n)
{
    if (!store_valid || read_pos + n > store_len) return -1;
    memcpy(buf, store + read_pos, n);
    read_pos += n;
    return 0;
}
void hal_storage_clear(void) { store_valid = 0; store_len = 0; }
