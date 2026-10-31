#ifndef TELECO_HAL_TIMEBASE_H
#define TELECO_HAL_TIMEBASE_H

#include <stdint.h>

/* Milliseconds since power-up (wraps after ~49 days; the core only uses differences). */
uint32_t hal_millis(void);

#endif
