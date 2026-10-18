#!/usr/bin/env python3
"""Differential test driver for core/numeric.

Generates reference vectors with Python's decimal module (16 digits,
round-half-even) and runs them through the C runner (dec_difftest).

The generators deliberately aim at the hard cases: exact rounding ties,
exponent-alignment boundaries, catastrophic cancellation, overflow/underflow
edges, perfect squares and over-long input strings.

Reproduce a run with the same --seed; use --dump FILE to keep the vectors
(e.g. to pin a failing case under tests/regression/).
"""
import argparse
import random
import subprocess
import sys
from decimal import MAX_EMAX, MIN_EMIN, ROUND_HALF_EVEN, Context, Decimal

CTX = Context(prec=16, rounding=ROUND_HALF_EVEN, Emax=MAX_EMAX, Emin=MIN_EMIN, traps=[])
EXP_MAX, EXP_MIN = 99, -99
LO, HI = 10**15, 10**16 - 1

SPECIAL_MANTS = [
    LO, LO + 1, LO + 2, HI, HI - 1, HI - 2, 5 * 10**15, 5 * 10**15 + 1,
    9999999999999995, 1111111111111111, 2 * 10**15, 3 * 10**15, 7 * 10**15,
    1234567890123456, 9876543210987654,
]


# ---- operand helpers: an operand is (neg, mantissa, exponent), mantissa 0 = zero ----

def clamp_m(m):
    return max(LO, min(HI, m))


def clamp_e(e):
    return max(EXP_MIN, min(EXP_MAX, e))


def to_decimal(op):
    neg, m, e = op
    if m == 0:
        return Decimal(0)
    return Decimal((1 if neg else 0, tuple(int(c) for c in str(m)), e - 15))


def fmt(d):
    """Canonical format shared with dec_to_str()."""
    if d == 0:
        return "0"
    sign, digits, _ = d.as_tuple()
    s = "".join(str(x) for x in digits).ljust(16, "0")
    assert len(s) == 16, (d, s)
    return ("-" if sign else "") + s[0] + "." + s[1:] + "E" + "%+03d" % d.adjusted()


def rand_mant(rng):
    r = rng.random()
    if r < 0.55:
        return rng.randrange(LO, HI + 1)
    if r < 0.70:
        return rng.choice(SPECIAL_MANTS)
    if r < 0.85:  # few significant digits, like numbers people type
        k = rng.randint(1, 6)
        return int(str(rng.randrange(10 ** (k - 1), 10**k)).ljust(16, "0"))
    return int(rng.choice("1259") + "".join(rng.choice("0159") for _ in range(15)))


def rand_exp(rng):
    r = rng.random()
    if r < 0.40:
        return rng.randint(EXP_MIN, EXP_MAX)
    if r < 0.80:
        return rng.randint(-5, 5)
    if r < 0.90:
        return rng.choice([99, 98, -99, -98, 0, 1, -1])
    return rng.randint(-20, 20)


def rand_operand(rng, allow_zero=True):
    if allow_zero and rng.random() < 0.02:
        return (0, 0, 0)
    return (rng.random() < 0.5, rand_mant(rng), rand_exp(rng))


# ---- reference results ----

def finish(res):
    """res is already rounded to 16 digits by CTX; apply the exponent range."""
    if res == 0:
        return "0", "OK"
    adj = res.adjusted()
    if adj > EXP_MAX:
        return "0", "OVERFLOW"
    if adj < EXP_MIN:
        return "0", "UNDERFLOW"
    return fmt(res), "OK"


def reference(op, a, b):
    if op == "add":
        return finish(CTX.add(a, b))
    if op == "sub":
        return finish(CTX.subtract(a, b))
    if op == "mul":
        return finish(CTX.multiply(a, b))
    if op == "div":
        if b == 0:
            return "0", ("DOMAIN" if a == 0 else "DIV0")
        return finish(CTX.divide(a, b))
    if op == "sqrt":
        if a < 0:
            return "0", "DOMAIN"
        return finish(CTX.sqrt(a))
    raise ValueError(op)


# ---- generators: each returns (a, b) operand tuples ----

def gen_addsub(rng):
    a = rand_operand(rng)
    k = rng.random()
    neg, m, e = a
    if m == 0 or k < 0.30:
        return a, rand_operand(rng)
    if k < 0.55:  # exponent alignment around the 16/18-digit boundaries
        return a, (rng.random() < 0.5, rand_mant(rng), clamp_e(e - rng.randint(-3, 25)))
    if k < 0.80:  # catastrophic cancellation
        if rng.random() < 0.7:
            return a, (rng.random() < 0.5, clamp_m(m + rng.randint(-5, 5)), e)
        return (neg, LO + rng.randint(0, 5), e), (rng.random() < 0.5, HI - rng.randint(0, 5), clamp_e(e - 1))
    if k < 0.92:  # exactly half an ulp: rounding ties
        return a, (rng.random() < 0.5, 5 * 10**15, clamp_e(e - 16))
    return a, (not neg, m, e)  # a - a or a + (-a)


def gen_mul(rng):
    k = rng.random()
    a = rand_operand(rng)
    if k < 0.45:
        return a, rand_operand(rng)
    if k < 0.65:  # ties: odd mantissa times 5
        return a, (rng.random() < 0.5, 5 * 10**15, rng.choice([0, -1, 1]))
    # product exponent near the overflow/underflow edges
    target = rng.choice([99, 100, 98, -99, -100, -98])
    ea = rng.randint(-99, 99)
    return (rng.random() < 0.5, rand_mant(rng), ea), (rng.random() < 0.5, rand_mant(rng), clamp_e(target - ea))


def gen_div(rng):
    k = rng.random()
    a = rand_operand(rng)
    if k < 0.40:
        return a, rand_operand(rng)
    if k < 0.60:  # divide by small integers: ties and repeating decimals
        d = rng.choice([2, 3, 4, 5, 6, 7, 8, 9])
        return a, (rng.random() < 0.5, d * 10**15, rng.randint(-3, 3))
    if k < 0.70:
        return a, a  # x / x
    target = rng.choice([99, 100, 98, -99, -100, -98])
    eb = rng.randint(-99, 99)
    return (rng.random() < 0.5, rand_mant(rng), clamp_e(target + eb)), (rng.random() < 0.5, rand_mant(rng), eb)


def gen_sqrt(rng):
    k = rng.random()
    if k < 0.45:
        return (False, rand_mant(rng), rand_exp(rng)), None
    if k < 0.85:  # perfect squares and their neighbours
        n = rng.randrange(1, 10**8) ** 2 + rng.choice([0, 0, 1, -1])
        n = max(n, 1)
        s = str(n)
        e = clamp_e(len(s) - 1 + rng.randint(-40, 40))
        return (False, int(s.ljust(16, "0")), e), None
    if k < 0.95:
        return (True, rand_mant(rng), rand_exp(rng)), None  # negative -> DOMAIN
    return (0, 0, 0), None


def gen_parse(rng):
    nd = rng.randint(1, 40)
    digits = "".join(rng.choice("0123456789") for _ in range(nd))
    if rng.random() < 0.35:  # build exact ties and just-above/below-tie strings
        head = str(rng.randrange(LO, HI + 1))
        tail = "5" + "0" * rng.randint(0, 18)
        if rng.random() < 0.5:
            tail = tail[:-1] + rng.choice("0123456789") if len(tail) > 1 else tail + rng.choice("123456789")
        digits = head + tail
    if rng.random() < 0.7:
        pos = rng.randint(0, len(digits))
        s = digits[:pos] + "." + digits[pos:]
    else:
        s = digits
    s = rng.choice(["", "", "+", "-"]) + s
    if rng.random() < 0.4:
        ex = rng.randint(-130, 130)
        s += rng.choice("eE") + ("+" if ex >= 0 and rng.random() < 0.3 else "") + str(ex)
    return s


GENERATORS = {
    "add": gen_addsub,
    "sub": gen_addsub,
    "mul": gen_mul,
    "div": gen_div,
    "sqrt": gen_sqrt,
}


def build_vectors(seed, count):
    rng = random.Random(seed)
    lines = ["# generated by run_diff.py seed=%d count=%d" % (seed, count)]
    for op in ("add", "sub", "mul", "div", "sqrt"):
        gen = GENERATORS[op]
        for _ in range(count):
            a, b = gen(rng)
            da = to_decimal(a)
            db = to_decimal(b) if b is not None else None
            want, status = reference(op, da, db)
            lines.append("%s %s %s %s %s" % (op, fmt(da), fmt(db) if db is not None else "-", want, status))
    for _ in range(count):
        s = gen_parse(rng)
        want, status = finish(CTX.plus(Decimal(s)))
        lines.append("parse %s - %s %s" % (s, want, status))
    return "\n".join(lines) + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--runner", help="path to the dec_difftest executable")
    ap.add_argument("--count", type=int, default=20000, help="vectors per operation")
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--dump", help="also write the generated vectors to this file")
    args = ap.parse_args()

    text = build_vectors(args.seed, args.count)
    if args.dump:
        with open(args.dump, "w") as f:
            f.write(text)
    if not args.runner:
        if not args.dump:
            sys.stdout.write(text)
        return 0

    print("differential test: seed=%d, %d vectors per operation" % (args.seed, args.count))
    sys.stdout.flush()
    proc = subprocess.run([args.runner, "-"], input=text.encode())
    if proc.returncode != 0:
        print("FAILED (reproduce with --seed %d --count %d)" % (args.seed, args.count))
    return proc.returncode


if __name__ == "__main__":
    sys.exit(main())
