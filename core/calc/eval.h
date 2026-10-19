#ifndef TELECO_CALC_EVAL_H
#define TELECO_CALC_EVAL_H

#include "parser/program.h"
#include "state/ctx.h"

#define VM_BUDGET 4000000u

/* Run a compiled program. On success *out holds the result (not stored to Ans). */
calc_err_t eval_prog(ctx_t *c, const prog_t *p, val_t *out);

/* Compile + evaluate text; on success sets Ans/PreAns (like pressing "="). */
calc_err_t calc_eval_text(ctx_t *c, const char *text, val_t *out);

/* Evaluate the token stream (from the editor), same effect as calc_eval_text. */
calc_err_t calc_eval_tokens(ctx_t *c, const tok_t *t, int n, val_t *out);

#endif
