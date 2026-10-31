/* Freestanding <stdio.h>: only snprintf with %d, %s and width / '-' flags. */
#ifndef TELECO_WEB_STDIO_H
#define TELECO_WEB_STDIO_H
#include <stddef.h>
int snprintf(char *buf, size_t cap, const char *fmt, ...);
#endif
