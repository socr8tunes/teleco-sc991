#include "parser/tokinfo.h"
#include "units/units.h"
#include "stats/stat.h"
#include <string.h>

#define S_SIGMA_X "\x87x"
#define S_SIGMA_Y "\x87y"
#define S_XBAR "\x93"
#define S_YBAR "\x94"
#define S_TIMES_ "\x82"
#define S_DIV_ "\x83"

int tok_opens_paren(uint8_t t)
{
    return t == T_LP || t == T_LOGB || (t >= T_SIN && t <= T_NEGB);
}

uint8_t tok_struct_end(uint8_t t)
{
    switch (t) {
    case T_FRAC: return T_FRACEND;
    case T_MFRAC: return T_MEND;
    case T_SQRT: return T_SQEND;
    case T_ROOT: return T_ROOTEND;
    case T_POW: return T_PEND;
    default: return 0;
    }
}

int tok_is_struct_end(uint8_t t)
{
    return t == T_FRACEND || t == T_MEND || t == T_SQEND || t == T_ROOTEND || t == T_PEND;
}

int tok_is_struct_sep(uint8_t t)
{
    return t == T_FRACSEP || t == T_MSEP1 || t == T_MSEP2 || t == T_ROOTSEP;
}

int tok_is_digit(uint8_t t)
{
    return (t >= T_D0 && t <= T_D9) || t == T_DOT;
}

int tok_struct_match(const tok_t *t, int n, int i)
{
    int depth = 0, k;
    for (k = i; k < n; k++) {
        if (tok_struct_end(t[k].t)) depth++;
        else if (tok_is_struct_end(t[k].t)) {
            if (--depth == 0) return k;
        }
    }
    return n;
}

int tok_autoclose(const tok_t *t, int n, tok_t *out, int cap)
{
    /* stack entries: 0 = structure marker, T_RP / T_RBRK = pending closer */
    uint8_t st[MAX_TOKENS];
    int sp = 0, o = 0, i;
#define PUT(tt, aa) do { if (o >= cap) return -1; out[o++] = mk_tok((uint8_t)(tt), (uint8_t)(aa)); } while (0)
    for (i = 0; i < n; i++) {
        uint8_t k = t[i].t;
        if (tok_struct_end(k)) {
            if (sp >= MAX_TOKENS) return -1;
            st[sp++] = 0;
        } else if (tok_is_struct_sep(k) || tok_is_struct_end(k)) {
            while (sp > 0 && st[sp - 1] != 0) PUT(st[--sp], 0);
            if (tok_is_struct_end(k) && sp > 0) sp--;
        } else if (tok_opens_paren(k)) {
            if (sp >= MAX_TOKENS) return -1;
            st[sp++] = T_RP;
        } else if (k == T_LBRK) {
            if (sp >= MAX_TOKENS) return -1;
            st[sp++] = T_RBRK;
        } else if (k == T_RP || k == T_RBRK) {
            if (sp > 0 && st[sp - 1] == k) sp--;
        }
        PUT(k, t[i].a);
    }
    while (sp > 0) {
        if (st[sp - 1] != 0) PUT(st[sp - 1], 0);
        sp--;
    }
#undef PUT
    return o;
}

static const char *const STATNAME[] = {
    "n", S_SIGMA_X, S_SIGMA_X "\x01" "2\x02", S_XBAR, "\x95x", "sx", S_SIGMA_Y,
    S_SIGMA_Y "\x01" "2\x02", S_YBAR, "\x95y", "sy", S_SIGMA_X "y", "a", "b", "c", "r",
    "minX", "maxX", "minY", "maxY", "Q1", "Med", "Q3",
    S_SIGMA_X "\x01" "3\x02", S_SIGMA_X "\x01" "4\x02", S_SIGMA_X "\x01" "2\x02y"
};

static const char *fn_label(uint8_t t)
{
    switch (t) {
    case T_SIN: return "sin("; case T_COS: return "cos("; case T_TAN: return "tan(";
    case T_ASIN: return "sin\x01-1\x02("; case T_ACOS: return "cos\x01-1\x02("; case T_ATAN: return "tan\x01-1\x02(";
    case T_SINH: return "sinh("; case T_COSH: return "cosh("; case T_TANH: return "tanh(";
    case T_ASINH: return "sinh\x01-1\x02("; case T_ACOSH: return "cosh\x01-1\x02("; case T_ATANH: return "tanh\x01-1\x02(";
    case T_LOG: return "log("; case T_LN: return "ln("; case T_EXP: return "e\x01^\x02("; case T_EXP10: return "10\x01^\x02(";
    case T_ABS: return "Abs("; case T_CBRT: return "\x01" "3\x02" "\x80("; case T_INT: return "Int("; case T_INTG: return "Intg(";
    case T_RND: return "Rnd("; case T_RANINT: return "RanInt("; case T_POL: return "Pol("; case T_REC: return "Rec(";
    case T_ARG: return "Arg("; case T_CONJ: return "Conjg("; case T_RE: return "Re("; case T_IM: return "Im(";
    case T_SIGMA: return "\x87("; case T_DDX: return "d/dx("; case T_INTEGRAL: return "\x86(";
    case T_DET: return "Det("; case T_TRN: return "Trn("; case T_IDEN: return "Iden("; case T_DOTP: return "Dot(";
    case T_CROSSP: return "Cross("; case T_ANGLEV: return "Angle("; case T_UNITV: return "UnitV(";
    case T_SOLVE: return "Solve("; case T_VCTLIT: return "Vct("; case T_YHAT: return "y^("; case T_XHAT: return "x^(";
    case T_CONSTF: return "Const("; case T_CONVF: return "Conv("; case T_DMSF: return "DMS(";
    case T_QUAD: return "Quad("; case T_CUBIC: return "Cubic("; case T_SIMUL2: return "Simul2("; case T_SIMUL3: return "Simul3(";
    case T_NORMP: return "NormP("; case T_NORMQ: return "NormQ("; case T_NORMR: return "NormR(";
    case T_GCD: return "GCD("; case T_LCM: return "LCM("; case T_QUOT: return "Quot("; case T_REM: return "Rem(";
    case T_PFACT: return "PFact("; case T_BINPD: return "BinomPD("; case T_BINCD: return "BinomCD(";
    case T_POIPD: return "PoisPD("; case T_POICD: return "PoisCD("; case T_INVN: return "InvN(";
    case T_PROD: return "\x9C(";
    case T_NOT: return "Not("; case T_NEGB: return "Neg(";
    case T_LOGB: return "log\x01" "b\x02(";
    default: return 0;
    }
}

const char *tok_label(tok_t t, char *buf, int cap)
{
    static const char VN[] = "ABCDEFXYM";
    uint8_t k = t.t;
    const char *f;
    if (cap < 24) return "?";
    if (k >= T_D0 && k <= T_D9) { buf[0] = (char)('0' + (k - T_D0)); buf[1] = 0; return buf; }
    if (k >= T_DA && k <= T_DF) { buf[0] = (char)('A' + (k - T_DA)); buf[1] = 0; return buf; }
    if ((f = fn_label(k)) != 0) return f;
    switch (k) {
    case T_BASEPFX: return t.a == 2 ? "0b" : t.a == 8 ? "0o" : t.a == 16 ? "0x" : "0d";
    case T_DOT: return ".";
    case T_EE: return "E";
    case T_NEG: return "-";
    case T_ADD: return "+"; case T_SUB: return "-"; case T_MUL: return S_TIMES_; case T_DIV: return S_DIV_;
    case T_LP: return "("; case T_RP: return ")"; case T_COMMA: return ","; case T_LBRK: return "["; case T_RBRK: return "]"; case T_SEMI: return ";";
    case T_FACT: return "!"; case T_SQ: return "\x01" "2\x02"; case T_CUBE: return "\x01" "3\x02";
    case T_RECIP: return "\x01-1\x02"; case T_PERCENT: return "%";
    case T_DEGSYM: return "\x01\x84\x02"; case T_RADSYM: return "\x01r\x02"; case T_GRADSYM: return "\x01g\x02";
    case T_DMS_D: return "\x84"; case T_DMS_M: return "'"; case T_DMS_S: return "\"";
    case T_NPR: return "P"; case T_NCR: return "C";
    case T_AND: return " and "; case T_OR: return " or "; case T_XOR: return " xor "; case T_XNOR: return " xnor ";
    case T_EQ: return "="; case T_COLON: return ":";
    case T_PI: return "\x81"; case T_E: return "e"; case T_I: return "i"; case T_ANS: return "Ans";
    case T_PREANS: return "PreAns"; case T_RAN: return "Ran#";
    case T_DANGLE: return "\x89";
    case T_STO: {
        static const char VNS[] = "ABCDEFXYM";
        int o = 0;
        buf[o++] = '\x85';
        if (t.a < 9) buf[o++] = VNS[t.a];
        else if (t.a >= 16 && t.a < 20) { const char *m = "Mat"; int q; for (q = 0; m[q]; q++) buf[o++] = m[q]; buf[o++] = (char)('A' + (t.a - 16)); }
        else if (t.a >= 24 && t.a < 28) { const char *m = "Vct"; int q; for (q = 0; m[q]; q++) buf[o++] = m[q]; buf[o++] = (char)('A' + (t.a - 24)); }
        buf[o] = 0;
        return buf;
    }
    case T_CONST: {
        const const_info_t *ci = const_get(t.a);
        return ci ? ci->symbol : "?";
    }
    case T_CONV: {
        const conv_info_t *cv = conv_get(t.a);
        int o = 0;
        const char *p;
        if (!cv) return "?";
        buf[o++] = '\x85';
        for (p = cv->to; *p && o < cap - 1; p++) buf[o++] = *p;
        buf[o] = 0;
        return buf;
    }
    case T_STATVAR: return t.a < sizeof STATNAME / sizeof STATNAME[0] ? STATNAME[t.a] : "?";
    case T_VAR: buf[0] = t.a < 9 ? VN[t.a] : '?'; buf[1] = 0; return buf;
    case T_MAT: { static const char *const M[] = {"MatA", "MatB", "MatC", "MatD", "MatAns"}; return t.a < 5 ? M[t.a] : "Mat?"; }
    case T_VCT: { static const char *const V[] = {"VctA", "VctB", "VctC", "VctD", "VctAns"}; return t.a < 5 ? V[t.a] : "Vct?"; }
    default: return "?";
    }
}
