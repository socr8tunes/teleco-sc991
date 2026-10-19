#!/usr/bin/env python3
"""
Expression-level differential test.

Generates random whole expressions, evaluates them with mpmath at 60 digits and
compares with the calculator's raw 16-digit result. Because the calculator rounds
every intermediate result to 16 digits, the accepted error is scaled by the largest
intermediate magnitude seen while evaluating (a standard condition-number bound).

  run_expr_diff.py --cli build/teleco_cli [--count N] [--seed S] [--dump]
"""
import argparse, random, subprocess, sys
from mpmath import mp, mpf, sin, cos, tan, atan, asin, acos, sinh, cosh, tanh, log, exp, sqrt, cbrt, pi, floor, fabs

mp.dps = 60
LIMIT = mpf(10) ** 99


class Bad(Exception):
    pass


def q(x):
    """round to 16 significant digits (what the calculator keeps after every step)"""
    if x == 0:
        return x
    return mpf(mp.nstr(x, 16, min_fixed=mpf("-inf"), max_fixed=mpf("inf")))


class Node:
    """text: calculator syntax.
    val: high precision value.
    up / dn: the value when every step is rounded to 16 digits after being nudged up / down
    by two units of the last digit; their spread measures how ill-conditioned the expression is."""

    def __init__(self, text, val, up=None, dn=None, kids=()):
        self.text, self.val = text, val
        self.mag = max([fabs(val)] + [k.mag for k in kids])   # largest intermediate magnitude
        self.up = q(val * (1 + NUDGE)) if up is None else up
        self.dn = q(val * (1 - NUDGE)) if dn is None else dn


NUDGE = mpf("1e-15")


def nq(x, sign):
    return q(x * (1 + sign * NUDGE))


def leaf(rnd):
    k = rnd.random()
    if k < 0.40:
        v = rnd.randint(1, 99)
        return Node(str(v), mpf(v), mpf(v), mpf(v))
    if k < 0.5:
        v = rnd.randint(1, 10 ** rnd.randint(3, 12))   # large integers exercise exact surd / int64 paths
        return Node(str(v), mpf(v), mpf(v), mpf(v))
    if k < 0.8:
        s = "%d.%d" % (rnd.randint(0, 99), rnd.randint(1, 9999))
        return Node(s, mpf(s), mpf(s), mpf(s))
    if k < 0.9:
        return Node("pi", +pi)
    v = q(exp(1))   # the calculator's e is the 16-digit constant
    return Node("e", v, v, v)


def check(v):
    if fabs(v) >= LIMIT or (v != 0 and fabs(v) < mpf(10) ** -90):
        raise Bad()
    return v


def wrap(t):
    return "(" + t + ")"


def ffun(f, x):
    if f in ("sin", "cos", "tan") and fabs(x) > 1000:
        raise Bad()
    if f in ("sin", "cos", "tan"):
        r = x / (pi / 12)
        if fabs(r - floor(r + mpf(1) / 2)) < mpf("1e-9"):
            raise Bad()   # special angles are exact in the calculator, not in a stepwise model
    if f == "tan" and fabs(cos(x)) < mpf("1e-3"):
        raise Bad()
    if f in ("sinh", "cosh", "exp") and fabs(x) > 100:
        raise Bad()
    if f in ("ln", "log") and x <= 0:
        raise Bad()
    if f == "sqrt" and x < 0:
        raise Bad()
    if f in ("asin", "acos") and fabs(x) > 1:
        raise Bad()
    v = {
        "sin": lambda: sin(x), "cos": lambda: cos(x), "tan": lambda: tan(x), "atan": lambda: atan(x),
        "sinh": lambda: sinh(x), "cosh": lambda: cosh(x), "tanh": lambda: tanh(x),
        "ln": lambda: log(x), "log": lambda: log(x, 10), "sqrt": lambda: sqrt(x), "exp": lambda: exp(x),
        "cbrt": lambda: cbrt(x) if x >= 0 else -cbrt(-x), "abs": lambda: fabs(x),
        "asin": lambda: asin(x), "acos": lambda: acos(x),
    }[f]()
    return check(v)


def gen(rnd, depth):
    if depth == 0 or rnd.random() < 0.15:
        return leaf(rnd)
    k = rnd.random()
    if k < 0.45:
        a, b = gen(rnd, depth - 1), gen(rnd, depth - 1)
        op = rnd.choice("+-*/")

        def f(x, y):
            if op == "+":
                return x + y
            if op == "-":
                return x - y
            if op == "*":
                return x * y
            if y == 0:
                raise Bad()
            return x / y
        return Node(wrap(a.text) + op + wrap(b.text), check(f(a.val, b.val)),
                    nq(check(f(a.up, b.up)), 1), nq(check(f(a.dn, b.dn)), -1), (a, b))
    if k < 0.55:
        a = gen(rnd, depth - 1)
        n = rnd.choice([2, 3, 4, -1, -2, 5])
        if a.val == 0 or a.up == 0 or a.dn == 0:
            raise Bad()
        return Node(wrap(a.text) + "^" + (str(n) if n >= 0 else "(" + str(n) + ")"),
                    check(a.val ** n), nq(check(a.up ** n), 1), nq(check(a.dn ** n), -1), (a,))
    if k < 0.62:
        a, b = gen(rnd, depth - 1), gen(rnd, 0)
        if a.val <= 0 or a.up <= 0 or a.dn <= 0:
            raise Bad()
        return Node(wrap(a.text) + "^" + wrap(b.text), check(a.val ** b.val),
                    nq(check(a.up ** b.up), 1), nq(check(a.dn ** b.dn), -1), (a, b))
    a = gen(rnd, depth - 1)
    f = rnd.choice(["sin", "cos", "tan", "atan", "sinh", "cosh", "tanh", "ln", "log", "sqrt", "exp", "cbrt", "abs", "asin", "acos"])
    return Node(f + "(" + a.text + ")", ffun(f, a.val), nq(ffun(f, a.up), 1), nq(ffun(f, a.dn), -1), (a,))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cli", required=True)
    ap.add_argument("--count", type=int, default=3000)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--dump", action="store_true")
    a = ap.parse_args()
    rnd = random.Random(a.seed)
    cases = []
    while len(cases) < a.count:
        try:
            n = gen(rnd, rnd.randint(1, 5))
        except (Bad, ZeroDivisionError, ValueError, OverflowError):
            continue
        if n.val == 0 or len(n.text) > 150:
            continue
        # skip ill-conditioned cases: rounding every step must not change the answer much
        n.spread = max(fabs(n.up - n.val), fabs(n.dn - n.val))
        if n.spread > fabs(n.val) * mpf("3e-14") or n.mag > 30 * fabs(n.val):
            continue
        cases.append(n)
    script = ":rad\n:raw\n" + "\n".join(n.text for n in cases) + "\n"
    out = subprocess.run([a.cli], input=script, capture_output=True, text=True, timeout=600).stdout.split("\n")
    out = [l for l in out if l != ""]
    if len(out) != len(cases):
        print("output line count mismatch: %d vs %d" % (len(out), len(cases)))
        sys.exit(2)
    bad = 0
    worst = 0
    for n, got in zip(cases, out):
        try:
            g = mpf(got)
        except Exception:
            bad += 1
            print("ERR  %s -> %s (expected %s)" % (n.text, got, mp.nstr(n.val, 17)))
            continue
        tol = n.spread + fabs(n.val) * mpf("1.5e-14")
        if fabs(g - n.val) > tol:
            bad += 1
            print("DIFF %s\n  got %s\n  ref %s" % (n.text, got, mp.nstr(n.val, 18)))
        else:
            worst = max(worst, fabs(g - n.val) / fabs(n.val))
    print("%d expressions, %d failures, worst relative error = %.2e" % (len(cases), bad, float(worst)))
    sys.exit(1 if bad else 0)


main()
