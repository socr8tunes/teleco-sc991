/*
 * Token classification and structure helpers shared by the editor, the
 * renderer and the key layer.
 */
#ifndef TELECO_PARSER_TOKINFO_H
#define TELECO_PARSER_TOKINFO_H

#include "parser/token.h"

/* Function / group openers that are closed by T_RP (T_LP, T_SIN .. T_NEGB, T_LOGB). */
int tok_opens_paren(uint8_t t);
/* Natural-structure begin token -> its end token (0 if t is not a structure begin). */
uint8_t tok_struct_end(uint8_t t);
int tok_is_struct_end(uint8_t t);
int tok_is_struct_sep(uint8_t t);
int tok_is_digit(uint8_t t);            /* T_D0..T_D9 or T_DOT */

/* Index of the end token matching the structure begin at i (n if unclosed). */
int tok_struct_match(const tok_t *t, int n, int i);

/*
 * Copy the editor's tokens to out, closing every open parenthesis group
 * (before separators / ends of a natural structure, and at the end).
 * Returns the new count, or -1 when cap would be exceeded.
 */
int tok_autoclose(const tok_t *t, int n, tok_t *out, int cap);

/* Text shown for a token. \x01 starts a raised small run, \x02 ends it. */
const char *tok_label(tok_t t, char *buf, int cap);

#endif
