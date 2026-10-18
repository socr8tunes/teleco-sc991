#ifndef TELECO_HAL_STORAGE_H
#define TELECO_HAL_STORAGE_H

#include <stddef.h>

/*
 * Retained state blob (Ans, variables, modes, replay history).
 * PC simulator: a file. MCU: retained RAM / backup domain (and/or flash).
 * Cleared only by an explicit reset, per the product's memory-protection rule.
 */

/* Returns 0 on success, non-zero if no valid blob exists or len mismatches. */
int hal_storage_load(void *buf, size_t len);

/* Returns 0 on success. */
int hal_storage_save(const void *buf, size_t len);

/* Erase the retained blob (used by the Reset function). */
void hal_storage_clear(void);

#endif /* TELECO_HAL_STORAGE_H */
