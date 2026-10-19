/* Calculator error codes shown to the user. */
#ifndef TELECO_CALC_ERRORS_H
#define TELECO_CALC_ERRORS_H

typedef enum calc_err {
    CE_OK = 0,
    CE_SYNTAX,     /* "Syntax ERROR"  : malformed expression                  */
    CE_MATH,       /* "Math ERROR"    : undefined / overflow / div by zero    */
    CE_STACK,      /* "Stack ERROR"   : expression too complex                */
    CE_ARG,        /* "Argument ERROR": wrong number or kind of arguments     */
    CE_DIM,        /* "Dimension ERROR": matrix / vector sizes                */
    CE_TIME,       /* "Time Out"      : iteration budget exhausted            */
    CE_RANGE,      /* "Range ERROR"   : value outside allowed range           */
    CE_NOSOL       /* "Can't Solve"   : solver found no root                  */
} calc_err_t;

const char *calc_err_name(calc_err_t e);

#endif
