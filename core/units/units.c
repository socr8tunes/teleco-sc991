#include <string.h>
#include "units/units.h"

static const const_info_t CONSTS[] = {
#include "constants_table.inc"
};
#define NCONST ((int)(sizeof CONSTS / sizeof CONSTS[0]))

int const_count(void) { return NCONST; }
const const_info_t *const_get(int i) { return (i >= 0 && i < NCONST) ? &CONSTS[i] : 0; }

dec_status const_value(int i, dec_t *out)
{
    if (i < 0 || i >= NCONST) { dec_zero(out); return DEC_DOMAIN; }
    return dec_from_str(out, CONSTS[i].value);
}

/* factor conversions: to = from * factor. Temperatures are special-cased by index. */
typedef struct conv_def {
    const char *from, *to, *factor;
} conv_def_t;

static const conv_def_t CONVS[] = {
    {"in", "cm", "2.54"}, {"cm", "in", "0.3937007874015748031496063"},
    {"ft", "m", "0.3048"}, {"m", "ft", "3.280839895013123359580052"},
    {"yd", "m", "0.9144"}, {"m", "yd", "1.093613298337707786526684"},
    {"mile", "km", "1.609344"}, {"km", "mile", "0.6213711922373339696174342"},
    {"n mile", "m", "1852"}, {"m", "n mile", "0.0005399568034557235421166307"},
    {"acre", "m2", "4046.8564224"}, {"m2", "acre", "0.0002471053814671653422482439"},
    {"gal (US)", "L", "3.785411784"}, {"L", "gal (US)", "0.2641720523581484153798999"},
    {"gal (UK)", "L", "4.54609"}, {"L", "gal (UK)", "0.2199692482990877875273037"},
    {"pc", "km", "30856775814913.672789"}, {"km", "pc", "0.00000000000003240779289444365023252326"},
    {"km/h", "m/s", "0.277777777777777778"}, {"m/s", "km/h", "3.59999999999999999712"},
    {"oz", "g", "28.349523125"}, {"g", "oz", "0.03527396194958041291567581"},
    {"lb", "kg", "0.45359237"}, {"kg", "lb", "2.204622621848775807229738"},
    {"atm", "Pa", "101325"}, {"Pa", "atm", "0.000009869232667160128300024673"},
    {"mmHg", "Pa", "133.322387415"}, {"Pa", "mmHg", "0.007500615758456563339512712"},
    {"hp", "kW", "0.74569987158227022192"}, {"kW", "hp", "1.341022089595027930870972"},
    {"kgf/cm2", "Pa", "98066.5"}, {"Pa", "kgf/cm2", "0.00001019716212977928242570093"},
    {"kgf m", "J", "9.80665"}, {"J", "kgf m", "0.1019716212977928242570093"},
    {"lbf/in2", "kPa", "6.8947572931683613"}, {"kPa", "lbf/in2", "0.1450377377302092159267372"},
    {"degF", "degC", "F"}, {"degC", "degF", "C"},
    {"cal", "J", "4.184"}, {"J", "cal", "0.2390057361376673040152964"},
};
#define NCONV ((int)(sizeof CONVS / sizeof CONVS[0]))

int conv_count(void) { return NCONV; }

const conv_info_t *conv_get(int i)
{
    static conv_info_t info;
    if (i < 0 || i >= NCONV) return 0;
    info.from = CONVS[i].from;
    info.to = CONVS[i].to;
    return &info;
}

dec_status conv_apply(int i, const dec_t *x, dec_t *out)
{
    dec_t f, t, k;
    dec_status s;
    if (i < 0 || i >= NCONV) { dec_zero(out); return DEC_DOMAIN; }
    if (CONVS[i].factor[0] == 'F') {            /* (F - 32) / 1.8 */
        dec_from_i64(&k, 32);
        s = dec_sub(&t, x, &k);
        if (s) { dec_zero(out); return s; }
        dec_from_str(&f, "1.8");
        return dec_div(out, &t, &f);
    }
    if (CONVS[i].factor[0] == 'C') {            /* C * 1.8 + 32 */
        dec_from_str(&f, "1.8");
        s = dec_mul(&t, x, &f);
        if (s) { dec_zero(out); return s; }
        dec_from_i64(&k, 32);
        return dec_add(out, &t, &k);
    }
    dec_from_str(&f, CONVS[i].factor);
    return dec_mul(out, x, &f);
}
