/*
 * Calculator tokens. The 2D editor stores a flat array of tokens; structures
 * (fractions, roots, powers) are delimited by begin/separator/end tokens so the
 * parser can recover the tree. The text lexer produces the same tokens.
 */
#ifndef TELECO_PARSER_TOKEN_H
#define TELECO_PARSER_TOKEN_H

#include <stdint.h>

typedef struct tok {
    uint8_t t;   /* enum tok_type */
    uint8_t a;   /* auxiliary: variable / constant / matrix index, conversion pair */
} tok_t;

enum tok_type {
    T_NONE = 0,
    /* number literal parts */
    T_D0, T_D1, T_D2, T_D3, T_D4, T_D5, T_D6, T_D7, T_D8, T_D9,
    T_DOT, T_EE, T_NEG,
    /* operators */
    T_ADD, T_SUB, T_MUL, T_DIV,
    T_LP, T_RP, T_COMMA, T_LBRK, T_RBRK, T_SEMI,
    /* natural structures */
    T_FRAC, T_FRACSEP, T_FRACEND,                 /* num / den */
    T_MFRAC, T_MSEP1, T_MSEP2, T_MEND,            /* whole, num, den */
    T_SQRT, T_SQEND,
    T_ROOT, T_ROOTSEP, T_ROOTEND,                 /* index, radicand */
    T_POW, T_PEND,                                /* superscript (editor): full expression inside */
    T_CARET,                                      /* '^' from text: operand follows, no end token */
    T_LOGB,                                       /* log_b(x):  T_LOGB base T_COMMA x T_RP */
    /* functions (each includes its opening parenthesis; closed by T_RP) */
    T_SIN, T_COS, T_TAN, T_ASIN, T_ACOS, T_ATAN,
    T_SINH, T_COSH, T_TANH, T_ASINH, T_ACOSH, T_ATANH,
    T_LOG, T_LN, T_EXP, T_EXP10, T_ABS, T_CBRT,
    T_INT, T_INTG, T_RND, T_RANINT, T_POL, T_REC,
    T_ARG, T_CONJ, T_RE, T_IM,
    T_SIGMA, T_DDX, T_INTEGRAL,
    T_DET, T_TRN, T_IDEN, T_DOTP, T_CROSSP, T_ANGLEV, T_UNITV,
    T_SOLVE, T_VCTLIT,
    T_NOT, T_NEGB,
    /* postfix */
    T_FACT, T_SQ, T_CUBE, T_RECIP, T_PERCENT,
    T_DEGSYM, T_RADSYM, T_GRADSYM,
    T_DMS_D, T_DMS_M, T_DMS_S,
    T_CONV,                                       /* a = conversion pair index */
    /* infix / statement */
    T_NPR, T_NCR, T_AND, T_OR, T_XOR, T_XNOR,
    T_EQ, T_COLON, T_STO,
    /* atoms */
    T_PI, T_E, T_I, T_ANS, T_PREANS, T_RAN,
    T_CONST,                                      /* a = constant index */
    T_STATVAR,                                    /* a = statistic id */
    T_VAR,                                        /* a = 0..8: A B C D E F X Y M */
    T_MAT,                                        /* a = 0..4: A B C D Ans */
    T_VCT,                                        /* a = 0..4 */
    T_LAST
};

#define VAR_A 0
#define VAR_B 1
#define VAR_C 2
#define VAR_D 3
#define VAR_E 4
#define VAR_F 5
#define VAR_X 6
#define VAR_Y 7
#define VAR_M 8
#define N_VARS 9

/* limits (all buffers are fixed size) */
#define MAX_TOKENS 160
#define MAX_CODE   256
#define MAX_CONSTS 40
#define EVAL_STACK 28
#define MAX_DEPTH  24

static inline tok_t mk_tok(uint8_t t, uint8_t a)
{
    tok_t k;
    k.t = t;
    k.a = a;
    return k;
}

#endif /* TELECO_PARSER_TOKEN_H */
