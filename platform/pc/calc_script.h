/* Line-oriented driver shared by the CLI and the test runners. */
#ifndef TELECO_PC_CALC_SCRIPT_H
#define TELECO_PC_CALC_SCRIPT_H

#include <stddef.h>
#include "calc/eval.h"
#include "calc/format.h"

typedef struct script {
    ctx_t ctx;
    int   exact;     /* show fractions / surds / pi          */
    int   ascii;     /* ASCII output                         */
    int   raw;       /* print the full 16-digit value        */
} script_t;

void script_init(script_t *s);
/*
 * Process one line. Returns 1 and fills out if the line produced output (a result,
 * an error name or a setting message), 0 for blank lines / comments / silent settings.
 */
int script_line(script_t *s, const char *line, char *out, size_t cap);

#endif
