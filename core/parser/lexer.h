/*
 * Text -> token stream. Produces exactly the tokens the 2D editor stores, so one
 * parser serves both the PC command line and the keypad.
 *
 *   sqrt(x)  or  √x        root(n,x)     frac(n,d)     mfrac(w,n,d)
 *   x^y  (operand follows)  x!  x²  x³   50%    30deg  1r  1g   1°30'15"
 *   sin( cos( ... ln( log(x) log(b,x)  Abs(  sum(f,a,b) ddx(f,x0) integral(f,a,b)
 *   Ans PreAns Ran# pi e i  A B C D E F X Y M    nPr nCr  and or xor xnor not(
 *   =  :  ->A (store)
 * Open groups are closed automatically at the end of the text.
 */
#ifndef TELECO_PARSER_LEXER_H
#define TELECO_PARSER_LEXER_H

#include "parser/token.h"
#include "calc/errors.h"

/* Returns CE_OK or CE_SYNTAX / CE_STACK. *n receives the token count. */
calc_err_t lex_text(const char *s, tok_t *out, int cap, int *n);

#endif
