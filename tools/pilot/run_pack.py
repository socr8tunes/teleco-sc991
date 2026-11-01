#!/usr/bin/env python3
"""Run the pilot question bank through the real calculator UI (teleco_sim) and check every answer.

  run_pack.py --sim build/teleco_sim [--write DIR]

Expected values come from mpmath / analytic results in questions.py, never from the calculator.
--write emits docs/pilot/test-pack.md (for testers) and results-record.md (this run).
"""
import re, argparse, datetime, pathlib, subprocess, sys
from mpmath import mp, mpf, mpc, sqrt, sin, cos, tan, atan, atan2, asin, acos, acosh, sinh, log, log10, exp, pi, e, erf, erfc, degrees, radians, findroot, nstr, fabs, fsum
from questions import QUESTIONS, ROOT
from keycomp import menu_pick

mp.dps = 40


def corr(xs, ys):
    n = len(xs)
    xs = [mpf(str(x)) for x in xs]; ys = [mpf(str(y)) for y in ys]
    sx, sy = sum(xs), sum(ys)
    sxy = sum(a * b for a, b in zip(xs, ys))
    sxx, syy = sum(a * a for a in xs), sum(b * b for b in ys)
    return (n * sxy - sx * sy) / sqrt((n * sxx - sx ** 2) * (n * syy - sy ** 2))


def yhat(x):
    from questions import XS, YS
    xs = [mpf(str(a)) for a in XS]; ys = [mpf(str(b)) for b in YS]
    n = len(xs)
    b = (n * sum(a * c for a, c in zip(xs, ys)) - sum(xs) * sum(ys)) / (n * sum(a * a for a in xs) - sum(xs) ** 2)
    a = (sum(ys) - b * sum(xs)) / n
    return a + b * x


def binpd(x, n, p):
    from mpmath import binomial
    return binomial(n, x) * mpf(str(p)) ** x * (1 - mpf(str(p))) ** (n - x)


def bincd(x, n, p): return fsum(binpd(k, n, p) for k in range(x + 1))


def poipd(x, lam):
    from mpmath import factorial
    return exp(-mpf(str(lam))) * mpf(str(lam)) ** x / factorial(x)


def poicd(x, lam): return fsum(poipd(k, lam) for k in range(x + 1))


def invn(p):
    from mpmath import erfinv
    return sqrt(2) * erfinv(2 * mpf(str(p)) - 1)


NS = dict(binpd=binpd, bincd=bincd, poipd=poipd, poicd=poicd, invn=invn, mp=mp, mpf=mpf, mpc=mpc, sqrt=sqrt, sin=sin, cos=cos, tan=tan, atan=atan, atan2=atan2, asin=asin, acos=acos, acosh=acosh,
          sinh=sinh, log=log, log10=log10, exp=exp, pi=pi, e=e, erf=erf, erfc=erfc, degrees=degrees, radians=radians,
          findroot=findroot, corr=corr, yhat=yhat, fsum=fsum)


def expected_value(q):
    if q["v"] is None:
        return None
    return eval(q["v"], dict(NS, __builtins__={"sum": sum, "range": range, "zip": zip, "abs": abs, "str": str}))


def run_sim(sim, script):
    r = subprocess.run([sim, "--quiet", "--text", "wait:2000 " + script], capture_output=True, text=True, timeout=60)
    if r.returncode != 0:
        return "<sim error: " + (r.stderr.strip() or r.stdout.strip()) + ">"
    for line in r.stdout.splitlines():
        if line.startswith("result:"):
            return line[7:].strip()
    return "<no result>"


def parse_number(s):
    s = s.replace(" ", "")
    try:
        if s.endswith("i"):
            return mpc(complex(s.replace("i", "j").replace("E", "e")))
        return mpf(s)
    except Exception:
        return None


def same(got, want, tol):
    g = parse_number(got)
    if g is None:
        return False
    err = fabs(g - want)
    scale = max(fabs(want), mpf(1e-12))
    return err <= tol * scale or err <= mpf("1e-12")


def fmt_expected(v):
    if isinstance(v, mpc):
        return nstr(v.real, 10) + ("+" if v.imag >= 0 else "") + nstr(v.imag, 10) + "i"
    return nstr(v, 10)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--sim", required=True)
    ap.add_argument("--write")
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args()

    rows, fails = [], 0
    for n, q in enumerate(QUESTIONS, 1):
        qid = f"Q{n:03d}"
        keys_full = (q["prefix"] + " " + q["keys"]).strip()
        keys_full = re.sub(r"\[\[MENU (\d+) (\d+)\]\]", lambda m: menu_pick(int(m.group(1)), int(m.group(2))), keys_full)
        if "MENU 8 3" not in keys_full or q["topic"] == "Regression" and "MENU 8 5" in keys_full:
            keys_full += " EXE"
        status, notes, shown = "PASS", [], ""
        want = expected_value(q)
        if q["exact"] is not None:
            shown = run_sim(a.sim, keys_full)
            if shown != q["exact"]:
                status = "FAIL"; notes.append(f"exact display: got '{shown}', expected '{q['exact']}'")
        if want is not None:
            dec = run_sim(a.sim, keys_full + " SD")
            if q["exact"] is None: shown = dec
            if not same(dec, want, q["tol"]):
                status = "FAIL"; notes.append(f"decimal: got '{dec}', expected {fmt_expected(want)}")
        if status == "FAIL": fails += 1
        rows.append(dict(id=qid, q=q, shown=shown, want=want, status=status, notes="; ".join(notes), keys=keys_full))
        if not a.quiet or status == "FAIL":
            print(f"{qid} {status:4} {q['level'][:12]:12} {q['topic']:18} {q['text'][:60]:60} {shown}" + (("  <-- " + "; ".join(notes)) if notes else ""))
    print(f"{len(rows)} questions, {fails} failures")
    if a.write:
        write_docs(pathlib.Path(a.write), rows, a.sim)
    return 1 if fails else 0


def write_docs(out, rows, sim):
    out.mkdir(parents=True, exist_ok=True)
    levels = []
    for r in rows:
        if r["q"]["level"] not in levels: levels.append(r["q"]["level"])
    t = ["# Teleco SC991 - pilot test pack", "",
         "Questions with independently verified answers. Testers: do each question using only the keys of the calculator,",
         "compare the displayed result with **Expected**, and record the outcome on the feedback form.",
         "Expected values were computed with mpmath (40 digits) or analytically - never with the calculator. The calculator",
         "shows 10 significant digits, so compare to 10 digits. `Exact` is what the natural display shows first; press S<>D for the decimal.",
         "The **Keys** column is one way to enter the question (the answer must not depend on how it is entered). Angle unit is degrees unless the question says radians.", ""]
    for lv in levels:
        t += [f"## {lv}", "", "| ID | Topic | Question | Expected | Exact display | Keys |", "|---|---|---|---|---|---|"]
        for r in rows:
            if r["q"]["level"] != lv: continue
            q = r["q"]
            exp_txt = fmt_expected(r["want"]) if r["want"] is not None else ""
            t.append(f"| {r['id']} | {q['topic']} | {q['text']} | {exp_txt} | {q['exact'] or ''} | `{r['keys']}` |".replace("|  |", "| |"))
        t.append("")
    (out / "test-pack.md").write_text("\n".join(t))

    ok = sum(1 for r in rows if r["status"] == "PASS")
    rec = ["# Pilot pack - automated results record", "",
           f"Run: {datetime.date.today().isoformat()} with `{pathlib.Path(sim).name}` built from this repository (host build, not hardware).",
           f"Result: **{ok} of {len(rows)} questions pass** through the calculator's key interface.", "",
           "This is the *software* record. It does not replace the hands-on pilot with students and teachers, and it is not a hardware result.", "",
           "| ID | Level | Topic | Status | Calculator showed | Notes |", "|---|---|---|---|---|---|"]
    for r in rows:
        rec.append(f"| {r['id']} | {r['q']['level']} | {r['q']['topic']} | {r['status']} | {r['shown']} | {r['notes']} |")
    (out / "results-record.md").write_text("\n".join(rec) + "\n")
    print(f"wrote {out}/test-pack.md and results-record.md")


if __name__ == "__main__":
    sys.exit(main())
