#!/usr/bin/env python3
"""Differential tests for the function library against mpmath (80 digits).

The expected value is the exact function value rounded once, half-even, to 16
significant digits -- i.e. a *correctly rounded* result. Vectors target the
hard cases: arguments near multiples of pi/2, near 1 for logs, tiny and huge
arguments, overflow/underflow edges, exact special angles and domain errors.

    run_fn_diff.py --runner build/fn_difftest --count 3000 --seed 1
"""
import argparse
import math
import random
import subprocess
import sys
from decimal import Decimal, Context, ROUND_HALF_EVEN, MAX_EMAX, MIN_EMIN, getcontext

import mpmath as mp

mp.mp.dps = 90
getcontext().prec = 120
CTX = Context(prec=16, rounding=ROUND_HALF_EVEN, Emax=MAX_EMAX, Emin=MIN_EMIN, traps=[])
LO, HI = 10**15, 10**16 - 1


def fmt(d):
    if d == 0:
        return "0"
    sign, digits, _ = d.as_tuple()
    s = "".join(str(x) for x in digits).ljust(16, "0")
    assert len(s) == 16, (d, s)
    return ("-" if sign else "") + s[0] + "." + s[1:] + "E" + "%+03d" % d.adjusted()


def finish(res):
    if res == 0:
        return "0", "OK"
    if res.adjusted() > 99:
        return "0", "OVERFLOW"
    if res.adjusted() < -99:
        return "0", "UNDERFLOW"
    return fmt(res), "OK"


def mp_to_dec(v):
    return CTX.plus(Decimal(mp.nstr(v, 85, min_fixed=-10**9, max_fixed=10**9)))


def D(s):
    return Decimal(s) if not isinstance(s, Decimal) else s


def M(d):
    return mp.mpf(str(d))


# ---------------- random operands (Decimals with <= 16 digits) ----------------

def rmant(rng):
    r = rng.random()
    if r < 0.6:
        return rng.randrange(LO, HI + 1)
    if r < 0.8:
        k = rng.randint(1, 6)
        return int(str(rng.randrange(10 ** (k - 1), 10 ** k)).ljust(16, "0"))
    return rng.choice([LO, LO + 1, HI, HI - 1, 5 * 10**15, 3141592653589793, 2718281828459045])


def mk(neg, m, e):
    return Decimal((1 if neg else 0, tuple(int(c) for c in str(m)), e - 15))


def rdec(rng, elo, ehi, neg=None):
    if neg is None:
        neg = rng.random() < 0.5
    return mk(neg, rmant(rng), rng.randint(elo, ehi))


def clampable(d):
    return d.adjusted() <= 99 and d.adjusted() >= -99


# ---------------- reference implementations ----------------

def ang_to_rad(x, mode):
    if mode == "D":
        return M(x) * mp.pi / 180
    if mode == "G":
        return M(x) * mp.pi / 200
    return M(x)


def rad_to_ang(v, mode):
    if mode == "D":
        return v * 180 / mp.pi
    if mode == "G":
        return v * 200 / mp.pi
    return v


def ref_trig(fn, x, mode):
    if abs(x) >= Decimal(10) ** 10:
        return "0", "DOMAIN"
    if mode in "DG":
        half, quarter = (Decimal(180), Decimal(90)) if mode == "D" else (Decimal(200), Decimal(100))
        rem = x % half  # exact, sign of x
        if rem == 0:
            return {"sin": ("0", "OK"), "tan": ("0", "OK"), "cos": None}[fn] or (
                ("1.000000000000000E+00" if (abs(x) / half) % 2 == 0 else "-1.000000000000000E+00"), "OK")
        if abs(rem) == quarter:
            if fn == "cos":
                return "0", "OK"
            if fn == "tan":
                return "0", "DOMAIN"
    v = ang_to_rad(x, mode)
    r = {"sin": mp.sin, "cos": mp.cos, "tan": mp.tan}[fn](v)
    return finish(mp_to_dec(r))


def ref_inv(fn, x, mode):
    xv = M(x)
    if fn in ("asin", "acos") and abs(x) > 1:
        return "0", "DOMAIN"
    if fn == "asin":
        r = mp.asin(xv)
    elif fn == "acos":
        r = mp.acos(xv)
    else:
        r = mp.atan(xv)
    if fn == "acos" and x == 1:
        return "0", "OK"
    if fn == "asin" and x == 0:
        return "0", "OK"
    if fn == "atan" and x == 0:
        return "0", "OK"
    return finish(mp_to_dec(rad_to_ang(r, mode)))


def ref_atan2(y, x, mode):
    if y == 0 and x == 0:
        return "0", "DOMAIN"
    if y == 0:
        return ("0", "OK") if x > 0 else finish(mp_to_dec(rad_to_ang(mp.pi, mode)))
    return finish(mp_to_dec(rad_to_ang(mp.atan2(M(y), M(x)), mode)))


def ref_unary(fn, x):
    xv = M(x)
    if fn == "exp":
        if x == 0:
            return "1.000000000000000E+00", "OK"
        return finish(mp_to_dec(mp.exp(xv)))
    if fn == "exp10":
        if x == int(x) and abs(x) < 1000:
            return finish(CTX.plus(Decimal(10) ** int(x)))
        return finish(mp_to_dec(mp.power(10, xv)))
    if fn == "ln":
        if x <= 0:
            return "0", "DOMAIN"
        if x == 1:
            return "0", "OK"
        return finish(mp_to_dec(mp.log(xv)))
    if fn == "log10":
        if x <= 0:
            return "0", "DOMAIN"
        if x == 1:
            return "0", "OK"
        return finish(mp_to_dec(mp.log10(xv)))
    if fn == "cbrt":
        if x == 0:
            return "0", "OK"
        v = mp.cbrt(abs(xv))
        return finish(mp_to_dec(-v if x < 0 else v))
    if fn == "sinh":
        return ("0", "OK") if x == 0 else finish(mp_to_dec(mp.sinh(xv)))
    if fn == "cosh":
        return finish(mp_to_dec(mp.cosh(xv)))
    if fn == "tanh":
        return ("0", "OK") if x == 0 else finish(mp_to_dec(mp.tanh(xv)))
    if fn == "asinh":
        return ("0", "OK") if x == 0 else finish(mp_to_dec(mp.asinh(xv)))
    if fn == "acosh":
        if x < 1:
            return "0", "DOMAIN"
        return ("0", "OK") if x == 1 else finish(mp_to_dec(mp.acosh(xv)))
    if fn == "atanh":
        if abs(x) >= 1:
            return "0", "DOMAIN"
        return ("0", "OK") if x == 0 else finish(mp_to_dec(mp.atanh(xv)))
    if fn == "fact":
        if x != int(x) or x < 0:
            return "0", "DOMAIN"
        n = int(x)
        if n > 69:
            return "0", "OVERFLOW"
        return finish(CTX.plus(Decimal(math.factorial(n))))
    if fn in ("floor", "trunc", "ceil", "frac"):
        if fn == "floor":
            return finish(CTX.plus(x.to_integral_value(rounding="ROUND_FLOOR")))
        if fn == "ceil":
            return finish(CTX.plus(x.to_integral_value(rounding="ROUND_CEILING")))
        t = x.to_integral_value(rounding="ROUND_DOWN")
        return finish(CTX.plus(t if fn == "trunc" else x - t))
    raise ValueError(fn)


def ref_binary(fn, a, b):
    if fn == "log":
        if a <= 0 or a == 1 or b <= 0:
            return "0", "DOMAIN"
        if b == 1:
            return "0", "OK"
        return finish(mp_to_dec(mp.log(M(b)) / mp.log(M(a))))
    if fn == "pow":
        if b == 0:
            return ("0", "DOMAIN") if a == 0 else ("1.000000000000000E+00", "OK")
        if a == 0:
            return ("0", "DIV0") if b < 0 else ("0", "OK")
        if b == int(b):
            n = int(b)
            return finish(CTX.plus(Decimal(a) ** n)) if abs(n) <= 400 and abs(a.adjusted() * n) < 3000 \
                else finish(mp_to_dec(mp.power(M(a), n)))
        if a < 0:
            return "0", "DOMAIN"
        return finish(mp_to_dec(mp.power(M(a), M(b))))
    if fn == "root":  # a-th root of b
        if a == 0:
            return "0", "DOMAIN"
        if b == 0:
            return ("0", "DIV0") if a < 0 else ("0", "OK")
        odd_int = a == int(a) and int(a) % 2 != 0
        if b < 0 and not odd_int:
            return "0", "DOMAIN"
        v = mp.power(abs(M(b)), 1 / M(a))
        return finish(mp_to_dec(-v if b < 0 else v))
    if fn in ("npr", "ncr"):
        if a != int(a) or b != int(b) or a < 0 or b < 0 or b > a or a >= Decimal(10) ** 10:
            return "0", "DOMAIN"
        n, k = int(a), int(b)
        lg = (math.lgamma(n + 1) - math.lgamma(n - k + 1)) / math.log(10) if fn == "npr" else \
            (math.lgamma(n + 1) - math.lgamma(k + 1) - math.lgamma(n - k + 1)) / math.log(10)
        if lg > 130:
            return "0", "OVERFLOW"
        v = math.perm(n, k) if fn == "npr" else math.comb(n, k)
        return finish(CTX.plus(Decimal(v)))
    raise ValueError(fn)


# ---------------- generators ----------------

def q16(fn):
    """Round whatever a generator returns to a valid 16-digit operand."""
    def wrapper(*a, **k):
        return CTX.plus(fn(*a, **k))
    wrapper.__name__ = fn.__name__
    return wrapper


@q16
def gen_exp(rng):
    r = rng.random()
    if r < 0.4:
        return rdec(rng, -3, 2)
    if r < 0.6:
        return mk(rng.random() < 0.5, rmant(rng), rng.randint(-20, -4))
    if r < 0.85:
        return Decimal(rng.randint(-240, 240)) + Decimal(rng.randrange(10**15)) / Decimal(10**15)
    return rng.choice([Decimal(0), Decimal(1), Decimal(-1), Decimal("230.2585092994046"),
                       Decimal("230.2585092994045"), Decimal("-227.9"), Decimal("-230"), Decimal("-231")])


@q16
def gen_pos(rng):
    r = rng.random()
    if r < 0.5:
        return rdec(rng, -99, 99, neg=False)
    if r < 0.7:
        return Decimal(1) + mk(rng.random() < 0.5, rmant(rng), -rng.randint(1, 15))
    if r < 0.8:
        return Decimal(10) ** rng.randint(-99, 99)
    if r < 0.9:
        return rdec(rng, -3, 3, neg=False)
    return rng.choice([Decimal(1), Decimal(2), Decimal(10), Decimal("0.5"), Decimal(0), Decimal(-1), Decimal("-3.5")])


@q16
def gen_trig_arg(rng, mode):
    r = rng.random()
    if r < 0.30:
        return rdec(rng, -2, 2)
    if r < 0.45:
        return rdec(rng, -20, -3)
    if r < 0.60:
        return rdec(rng, 3, 9)
    if r < 0.75:  # special angles
        q = {"D": 15, "G": 25, "R": 0}[mode]
        if q:
            return Decimal(q * rng.randint(-100, 100))
        return D(rng.choice(["0", "1", "-1", "3.141592653589793", "1.570796326794897", "6.283185307179586"]))
    if r < 0.85:  # near multiples of a quarter turn
        quarter = {"D": 90, "G": 100}.get(mode)
        if quarter:
            k = rng.randint(-1000, 1000)
            return Decimal(k * quarter) + mk(rng.random() < 0.5, rmant(rng), -rng.randint(3, 12))
        k = rng.randint(-1000, 1000)
        v = CTX.plus(Decimal(str(mp.nstr(mp.pi / 2 * k, 40))))
        return v + rng.choice([0, 0, 1, -1]) * Decimal(10) ** (v.adjusted() - 15)
    if r < 0.95:
        return rdec(rng, 4, 9)
    return rng.choice([Decimal(0), Decimal("9999999999"), Decimal("-9999999999"), Decimal("10000000000"), Decimal("1E12")])


@q16
def gen_unit(rng):
    r = rng.random()
    if r < 0.6:
        return mk(rng.random() < 0.5, rmant(rng), rng.randint(-3, -1))
    if r < 0.8:
        return mk(rng.random() < 0.5, rmant(rng), rng.randint(-15, -4))
    if r < 0.9:
        return Decimal(1) - mk(False, rmant(rng), -rng.randint(1, 15))
    return rng.choice([Decimal(1), Decimal(-1), Decimal(0), Decimal("1.0000000000000001"), Decimal("-1.1"), Decimal("0.5")])


@q16
def gen_hyp_arg(rng, fn):
    if fn in ("sinh", "cosh", "tanh"):
        r = rng.random()
        if r < 0.5:
            return rdec(rng, -3, 1)
        if r < 0.7:
            return rdec(rng, -15, -4)
        if r < 0.9:
            return Decimal(rng.randint(-230, 230)) + Decimal(rng.randrange(10**8)) / Decimal(10**8)
        return rng.choice([Decimal(0), Decimal("230.9"), Decimal("231"), Decimal("-231"), Decimal("45"), Decimal("-45"), Decimal("800")])
    if fn == "asinh":
        return rdec(rng, -15, 30) if rng.random() < 0.95 else Decimal(0)
    if fn == "acosh":
        r = rng.random()
        if r < 0.4:
            return Decimal(1) + mk(False, rmant(rng), -rng.randint(1, 15))
        if r < 0.9:
            return rdec(rng, 0, 40, neg=False)
        return rng.choice([Decimal(1), Decimal("0.5"), Decimal(0), Decimal(-2)])
    return gen_unit(rng)  # atanh


def emit(lines, fn, mode, a, b, res):
    lines.append("%s %s %s %s %s %s" % (fn, mode, fmt(a), fmt(b) if b is not None else "-", res[0], res[1]))


def build(seed, count):
    rng = random.Random(seed)
    lines = ["# generated by run_fn_diff.py seed=%d count=%d" % (seed, count)]
    for _ in range(count):
        x = gen_exp(rng)
        emit(lines, "exp", "-", x, None, ref_unary("exp", x))
        x = gen_exp(rng) if rng.random() < 0.5 else rdec(rng, -3, 2)
        emit(lines, "exp10", "-", x, None, ref_unary("exp10", x))
        for fn in ("ln", "log10"):
            x = gen_pos(rng)
            emit(lines, fn, "-", x, None, ref_unary(fn, x))
        x = rdec(rng, -99, 99)
        emit(lines, "cbrt", "-", x, None, ref_unary("cbrt", x))
        # log with random base
        a = gen_pos(rng) if rng.random() < 0.5 else rng.choice([Decimal(2), Decimal(10), Decimal("0.5"), Decimal(3)])
        b = gen_pos(rng)
        emit(lines, "log", "-", a, b, ref_binary("log", a, b))
        # pow
        r = rng.random()
        if r < 0.45:
            a = rdec(rng, -3, 3)
            b = Decimal(rng.randint(-40, 40))
        elif r < 0.8:
            a = rdec(rng, -3, 3, neg=False)
            b = rdec(rng, -2, 1)
        elif r < 0.9:
            a = rdec(rng, -20, 20)
            b = Decimal(rng.randint(-5000, 5000))
        else:
            a = rng.choice([Decimal(0), Decimal(2), Decimal(-2), Decimal(10), Decimal("0.1")])
            b = rng.choice([Decimal(0), Decimal(1), Decimal(-1), Decimal("0.5"), Decimal(100), Decimal(-100), Decimal(99), Decimal(300)])
        emit(lines, "pow", "-", a, b, ref_binary("pow", a, b))
        # roots
        n = rng.choice([Decimal(2), Decimal(3), Decimal(4), Decimal(5), Decimal(7), Decimal("2.5"), Decimal(-3), Decimal(0)])
        x = rdec(rng, -10, 10)
        emit(lines, "root", "-", n, x, ref_binary("root", n, x))
        # trig
        for fn in ("sin", "cos", "tan"):
            mode = rng.choice("DRG")
            x = gen_trig_arg(rng, mode)
            emit(lines, fn, mode, x, None, ref_trig(fn, x, mode))
        for fn in ("asin", "acos"):
            mode = rng.choice("DRG")
            x = gen_unit(rng)
            emit(lines, fn, mode, x, None, ref_inv(fn, x, mode))
        mode = rng.choice("DRG")
        x = rdec(rng, -15, 15) if rng.random() < 0.9 else Decimal(0)
        emit(lines, "atan", mode, x, None, ref_inv("atan", x, mode))
        y, x = rdec(rng, -5, 5), rdec(rng, -5, 5)
        if rng.random() < 0.08:
            y = Decimal(0)
        emit(lines, "atan2", mode, y, x, ref_atan2(y, x, mode))
        for fn in ("sinh", "cosh", "tanh", "asinh", "acosh", "atanh"):
            x = gen_hyp_arg(rng, fn)
            emit(lines, fn, "-", x, None, ref_unary(fn, x))
        # integer functions
        x = rdec(rng, -3, 17)
        for fn in ("floor", "trunc", "ceil", "frac"):
            emit(lines, fn, "-", x, None, ref_unary(fn, x))
        # factorial / combinatorics
        x = Decimal(rng.randint(-2, 72)) if rng.random() < 0.9 else rdec(rng, -1, 2)
        emit(lines, "fact", "-", x, None, ref_unary("fact", x))
        n = Decimal(rng.choice([rng.randint(0, 30), rng.randint(0, 400), rng.randint(0, 10**9), rng.randint(0, 10**10 + 5)]))
        k = Decimal(min(int(n), rng.choice([0, 1, 2, 3, 5, 12, rng.randint(0, 60)]))) if rng.random() < 0.95 else Decimal(rng.randint(-1, 500))
        emit(lines, "npr", "-", n, k, ref_binary("npr", n, k))
        emit(lines, "ncr", "-", n, k, ref_binary("ncr", n, k))
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--runner")
    ap.add_argument("--count", type=int, default=2000, help="loop iterations (about 40 vectors each)")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--dump")
    args = ap.parse_args()
    text = build(args.seed, args.count)
    if args.dump:
        with open(args.dump, "w") as f:
            f.write(text)
    if not args.runner:
        if not args.dump:
            sys.stdout.write(text)
        return 0
    print("function differential test: seed=%d, %d iterations" % (args.seed, args.count))
    sys.stdout.flush()
    proc = subprocess.run([args.runner, "-"], input=text.encode())
    if proc.returncode != 0:
        print("FAILED (reproduce with --seed %d --count %d)" % (args.seed, args.count))
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
