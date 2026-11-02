#include "calc/errors.h"

const char *calc_err_name(calc_err_t e)
{
    switch (e) {
    case CE_OK:     return "OK";
    case CE_SYNTAX: return "Syntax ERROR";
    case CE_MATH:   return "Math ERROR";
    case CE_STACK:  return "Stack ERROR";
    case CE_ARG:    return "Argument ERROR";
    case CE_DIM:    return "Dimension ERROR";
    case CE_TIME:   return "Time Out";
    case CE_RANGE:  return "Range ERROR";
    case CE_NOSOL:  return "Can't Solve";
    case CE_INCONS: return "No Solution";
    case CE_INFIN:  return "Infinite Solutions";
    }
    return "ERROR";
}
