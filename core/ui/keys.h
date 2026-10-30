/*
 * Logical keys of the Teleco SC991 (index = position in the key matrix layout:
 * rows of 6, then the 5-wide number pad). The platform maps physical scan codes
 * to these.
 */
#ifndef TELECO_UI_KEYS_H
#define TELECO_UI_KEYS_H

enum key_id {
    K_SHIFT, K_ALPHA, K_LEFT, K_UP, K_DOWN, K_RIGHT,
    K_MENU, K_MODE, K_FRAC, K_SQRT, K_SQ, K_POW,
    K_LOG, K_LN, K_NEG, K_DMS, K_HYP, K_SIN,
    K_COS, K_TAN, K_STO, K_SD, K_ENG, K_MPLUS,
    K_LPAREN, K_RPAREN, K_XINV, K_FACT, K_EE,
    K_7, K_8, K_9, K_DEL, K_AC,
    K_4, K_5, K_6, K_MUL, K_DIV,
    K_1, K_2, K_3, K_ADD, K_SUB,
    K_0, K_DOT, K_PI, K_ANS, K_EXE,
    K_COUNT
};

#endif
