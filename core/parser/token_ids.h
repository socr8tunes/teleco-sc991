/*
 * FROZEN persistent token IDs ("wire IDs").
 *
 * The replay history in the saved blob stores tokens as these IDs, not as enum tok_type values, so the enum in token.h
 * may be reordered or extended freely without invalidating anything a user (or a pilot unit) has saved.
 *
 * Rules (enforced by tests/unit/test_token_ids.c against tests/exam/frozen_token_ids.txt):
 *   - an ID, once released, is never reused or changed; a retired token keeps its ID reserved (mark it "retired" in
 *     frozen_token_ids.txt and keep the line);
 *   - new tokens take the next free ID inside their own group below (groups leave reserved gaps);
 *   - 0xF0-0xFE are reserved for future groups, 0xFF is never a valid ID.
 * Planned: T_QUARTIC (functions group) takes the next free function ID when it is merged; the inequality solver, TABLE and
 * CALC are expected to need no persisted token.
 * Auxiliary values (the a byte) are frozen in tests/exam/frozen_aux.txt: constants, conversions and statistics
 * variables may only be appended.
 */
#ifndef TELECO_PARSER_TOKEN_IDS_H
#define TELECO_PARSER_TOKEN_IDS_H

#include "parser/token.h"

#define TOK_WIRE_LIST(X) \
    X(T_NONE, 0x00) \
    /* literal parts: 0x01-0x1F, 11 free */ \
    X(T_D0, 0x01) \
    X(T_D1, 0x02) \
    X(T_D2, 0x03) \
    X(T_D3, 0x04) \
    X(T_D4, 0x05) \
    X(T_D5, 0x06) \
    X(T_D6, 0x07) \
    X(T_D7, 0x08) \
    X(T_D8, 0x09) \
    X(T_D9, 0x0A) \
    X(T_DA, 0x0B) \
    X(T_DB, 0x0C) \
    X(T_DC, 0x0D) \
    X(T_DD, 0x0E) \
    X(T_DE, 0x0F) \
    X(T_DF, 0x10) \
    X(T_BASEPFX, 0x11) \
    X(T_DOT, 0x12) \
    X(T_EE, 0x13) \
    X(T_NEG, 0x14) \
    /* operators and brackets: 0x20-0x2F, 6 free */ \
    X(T_ADD, 0x20) \
    X(T_SUB, 0x21) \
    X(T_MUL, 0x22) \
    X(T_DIV, 0x23) \
    X(T_LP, 0x24) \
    X(T_RP, 0x25) \
    X(T_COMMA, 0x26) \
    X(T_LBRK, 0x27) \
    X(T_RBRK, 0x28) \
    X(T_SEMI, 0x29) \
    /* natural structures: 0x30-0x4F, 16 free */ \
    X(T_FRAC, 0x30) \
    X(T_FRACSEP, 0x31) \
    X(T_FRACEND, 0x32) \
    X(T_MFRAC, 0x33) \
    X(T_MSEP1, 0x34) \
    X(T_MSEP2, 0x35) \
    X(T_MEND, 0x36) \
    X(T_SQRT, 0x37) \
    X(T_SQEND, 0x38) \
    X(T_ROOT, 0x39) \
    X(T_ROOTSEP, 0x3A) \
    X(T_ROOTEND, 0x3B) \
    X(T_POW, 0x3C) \
    X(T_PEND, 0x3D) \
    X(T_CARET, 0x3E) \
    X(T_LOGB, 0x3F) \
    /* functions (each includes its opening parenthesis): 0x50-0xBF, 46 free */ \
    X(T_SIN, 0x50) \
    X(T_COS, 0x51) \
    X(T_TAN, 0x52) \
    X(T_ASIN, 0x53) \
    X(T_ACOS, 0x54) \
    X(T_ATAN, 0x55) \
    X(T_SINH, 0x56) \
    X(T_COSH, 0x57) \
    X(T_TANH, 0x58) \
    X(T_ASINH, 0x59) \
    X(T_ACOSH, 0x5A) \
    X(T_ATANH, 0x5B) \
    X(T_LOG, 0x5C) \
    X(T_LN, 0x5D) \
    X(T_EXP, 0x5E) \
    X(T_EXP10, 0x5F) \
    X(T_ABS, 0x60) \
    X(T_CBRT, 0x61) \
    X(T_INT, 0x62) \
    X(T_INTG, 0x63) \
    X(T_RND, 0x64) \
    X(T_RANINT, 0x65) \
    X(T_POL, 0x66) \
    X(T_REC, 0x67) \
    X(T_ARG, 0x68) \
    X(T_CONJ, 0x69) \
    X(T_RE, 0x6A) \
    X(T_IM, 0x6B) \
    X(T_SIGMA, 0x6C) \
    X(T_DDX, 0x6D) \
    X(T_INTEGRAL, 0x6E) \
    X(T_DET, 0x6F) \
    X(T_TRN, 0x70) \
    X(T_IDEN, 0x71) \
    X(T_DOTP, 0x72) \
    X(T_CROSSP, 0x73) \
    X(T_ANGLEV, 0x74) \
    X(T_UNITV, 0x75) \
    X(T_SOLVE, 0x76) \
    X(T_VCTLIT, 0x77) \
    X(T_YHAT, 0x78) \
    X(T_XHAT, 0x79) \
    X(T_CONSTF, 0x7A) \
    X(T_CONVF, 0x7B) \
    X(T_DMSF, 0x7C) \
    X(T_QUAD, 0x7D) \
    X(T_CUBIC, 0x7E) \
    X(T_SIMUL2, 0x7F) \
    X(T_SIMUL3, 0x80) \
    X(T_SIMUL4, 0x81) \
    X(T_NORMP, 0x82) \
    X(T_NORMQ, 0x83) \
    X(T_NORMR, 0x84) \
    X(T_GCD, 0x85) \
    X(T_LCM, 0x86) \
    X(T_QUOT, 0x87) \
    X(T_REM, 0x88) \
    X(T_PFACT, 0x89) \
    X(T_BINPD, 0x8A) \
    X(T_BINCD, 0x8B) \
    X(T_POIPD, 0x8C) \
    X(T_POICD, 0x8D) \
    X(T_INVN, 0x8E) \
    X(T_PROD, 0x8F) \
    X(T_NOT, 0x90) \
    X(T_NEGB, 0x91) \
    /* postfix: 0xC0-0xCF, 4 free */ \
    X(T_FACT, 0xC0) \
    X(T_SQ, 0xC1) \
    X(T_CUBE, 0xC2) \
    X(T_RECIP, 0xC3) \
    X(T_PERCENT, 0xC4) \
    X(T_DEGSYM, 0xC5) \
    X(T_RADSYM, 0xC6) \
    X(T_GRADSYM, 0xC7) \
    X(T_DMS_D, 0xC8) \
    X(T_DMS_M, 0xC9) \
    X(T_DMS_S, 0xCA) \
    X(T_CONV, 0xCB) \
    /* infix and statement: 0xD0-0xDF, 7 free */ \
    X(T_NPR, 0xD0) \
    X(T_NCR, 0xD1) \
    X(T_AND, 0xD2) \
    X(T_OR, 0xD3) \
    X(T_XOR, 0xD4) \
    X(T_XNOR, 0xD5) \
    X(T_EQ, 0xD6) \
    X(T_COLON, 0xD7) \
    X(T_STO, 0xD8) \
    /* atoms: 0xE0-0xEF, 4 free */ \
    X(T_PI, 0xE0) \
    X(T_E, 0xE1) \
    X(T_I, 0xE2) \
    X(T_ANS, 0xE3) \
    X(T_PREANS, 0xE4) \
    X(T_RAN, 0xE5) \
    X(T_CONST, 0xE6) \
    X(T_STATVAR, 0xE7) \
    X(T_VAR, 0xE8) \
    X(T_MAT, 0xE9) \
    X(T_VCT, 0xEA) \
    X(T_DANGLE, 0xEB)

#define TOK_WIRE_INIT(n, id) [n] = (id),
static const uint8_t TOK_WIRE[T_LAST] = { TOK_WIRE_LIST(TOK_WIRE_INIT) };
#undef TOK_WIRE_INIT

#endif
