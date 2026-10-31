#ifndef TELECO_HAL_STORAGE_H
#define TELECO_HAL_STORAGE_H

#include <stddef.h>

/*
 * Retained state blob (Ans, variables, modes, replay history, statistics data).
 * PC simulator: a file. MCU: backup SRAM and/or flash pages (wear-levelled).
 *
 * The blob is written as a stream so the core never needs a second 16 KB buffer.
 * The previous blob stays valid until hal_storage_commit() succeeds, so a power
 * loss during a save cannot destroy the last good state.
 * Cleared only by an explicit reset, per the product's memory-protection rule.
 */

/* Start writing a blob of exactly `total` bytes. Returns 0 on success. */
int hal_storage_begin(size_t total);
/* Append bytes (sequential). Returns 0 on success. */
int hal_storage_write(const void *data, size_t len);
/* Make the new blob the valid one. Returns 0 on success. */
int hal_storage_commit(void);

/* Open the stored blob for reading; *len receives its size. Returns 0 if one exists. */
int hal_storage_open(size_t *len);
/* Read the next len bytes (sequential). Returns 0 on success. */
int hal_storage_read(void *buf, size_t len);

/* Erase the retained blob (used by the Reset function). */
void hal_storage_clear(void);

#endif
