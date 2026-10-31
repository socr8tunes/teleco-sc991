/* PC implementation of the HAL with a fake clock, scripted keypad and in-memory storage (tests, simulator). */
#ifndef TELECO_PC_HAL_PC_H
#define TELECO_PC_HAL_PC_H

#include <stddef.h>
#include <stdint.h>

void hal_pc_reset(void);                          /* clock 0, nothing held, display state cleared (storage kept) */
void hal_pc_set_keys(const uint8_t *keys, int n); /* keys physically held now */
void hal_pc_advance(uint32_t ms);
const uint8_t *hal_pc_framebuffer(void);          /* last presented frame */
int  hal_pc_present_count(void);
int  hal_pc_display_on(void);
int  hal_pc_sleep_count(void);
void hal_pc_set_battery_low(int v);
void hal_pc_storage_corrupt(size_t offset);       /* flip one byte of the stored blob */
size_t hal_pc_storage_size(void);
void hal_pc_storage_wipe(void);
/* make the next commit fail (power loss during save) */
void hal_pc_storage_fail_next_commit(void);

#endif
