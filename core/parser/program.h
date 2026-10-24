/*
 * Compiled expression: RPN instructions with fixed-size storage.
 * Opcodes are token types (T_ADD, T_SIN, ...) plus the extra ones below.
 */
#ifndef TELECO_PARSER_PROGRAM_H
#define TELECO_PARSER_PROGRAM_H

#include "parser/token.h"
#include "values/val.h"
#include "calc/errors.h"

enum {
    OP_CONST = 200,   /* push consts[x] (real)                               */
    OP_POP,           /* discard top (statement separator)                    */
    OP_UNEG,          /* unary minus                                          */
    OP_FN,            /* function: x = tok type, a = argc                     */
    OP_BLOCK_FN,      /* function with body block: x = tok, a = argc, y = body length */
    OP_DMS,           /* a = number of components given (1..3)                */
    OP_MATLIT         /* matrix literal: a = rows, x = cols, elements on the stack */
};

typedef struct ins {
    uint8_t  op;
    uint8_t  a;
    uint16_t x;
    uint16_t y;
} ins_t;

typedef struct prog {
    ins_t  code[MAX_CODE];
    num_t  consts[MAX_CONSTS];
    int    n;
    int    nconst;
    uint8_t has_eq;      /* contained '=' (equation) */
    uint8_t uses_x;      /* mentions variable X       */
} prog_t;

/* Token stream -> program. */
calc_err_t compile_tokens(const tok_t *toks, int ntok, prog_t *p);
/* Text -> program (lex + compile). */
calc_err_t compile_text(const char *s, prog_t *p);

#endif
