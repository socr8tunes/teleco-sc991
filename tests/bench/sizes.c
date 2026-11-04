/* Compiled for the 32-bit target by tools/check_budget.sh: exposes exact Cortex-M structure sizes as symbols
 * (read with llvm-nm -S) and enforces per-structure ceilings at compile time. */
#include "ui/app.h"
#include "ui/shell.h"
#include "parser/program.h"
char sz_app_t[sizeof(app_t)];
char sz_ctx_t[sizeof(ctx_t)];
char sz_prog_t[sizeof(prog_t)];
char sz_val_t[sizeof(val_t)];
char sz_num_t[sizeof(num_t)];
char sz_editor[sizeof(((app_t *)0)->ed)];
char sz_hist[sizeof(((app_t *)0)->hist)];
_Static_assert(sizeof(app_t) <= 20 * 1024, "app_t over its 20 KB budget");
_Static_assert(sizeof(prog_t) <= 8 * 1024, "prog_t over its 8 KB budget");
