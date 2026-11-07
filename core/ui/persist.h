/* Save / restore the calculator's retained state through hal/storage.h. */
#ifndef TELECO_UI_PERSIST_H
#define TELECO_UI_PERSIST_H

#include "ui/app.h"

/* Returns 0 on success. */
int persist_save(app_t *a);
/*
 * Returns 0 and restores the state if a valid blob (magic, version, size and CRC32
 * all match). On any mismatch the app is left untouched and non-zero is returned.
 */
int persist_load(app_t *a);

#endif
