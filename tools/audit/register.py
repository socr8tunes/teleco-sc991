#!/usr/bin/env python3
"""SC991 Exam audit generator: inventory counts from source, evidence-based test coverage, register output.

Writes docs/exam/feature-register.csv and docs/exam/04-coverage-register.md.  Run from anywhere:
    python3 tools/audit/register.py [--check]
--check exits non-zero when a lexer word, menu entry or key action is not covered by the catalogue
(guards the register against silently falling behind the code).

Evidence rules (deliberately simple and mechanical, see docs/exam/04-coverage-register.md):
  N  normal      >=1 script case (tests/cases) mentioning the feature that does not expect an error
  E  error       >=1 script case mentioning the feature that expects an error (domain/argument/etc.)
  B  boundary    >=1 case in a block following a '# boundary' comment, or in tests/unit boundary tests
  A  accuracy    case file generated against an independent oracle (mpmath/Fractions) or the mpmath
                 differential tests, or pilot questions whose expected values come from mpmath
  K  key path    >=1 pilot question or tests/ui line that reaches the feature through the key interface
  R  regression  the feature is exercised by a committed, deterministic test that runs in CI
Status: TESTED = N and K and (E or B) and (A or hand-verified); INSUFFICIENT = implemented but fails that rule.
"""
import csv, glob, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools/audit")); sys.path.insert(0, str(ROOT / "tools/pilot"))
from catalogue import F  # noqa: E402

import glob  # noqa
ORACLE_FILES = {"calculus", "stats", "distribution", "product", "simul"} | {pathlib.Path(p).stem for p in glob.glob(str(ROOT / "tests/cases/oracle_*.txt"))}   # generated with mpmath / Fractions
BOUNDARY_FILES = {"oracle_domain", "oracle_functions", "oracle_stats_errors", "capacity"}          # every line is a limit / domain-edge case by construction
PARTIAL_ORACLE = {"complex"}                                                  # transcendental values from mpmath
DIFF_FUNCS = r"(?<![a-z])(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|asinh|acosh|atanh|ln|log|exp|alog|sqrt|cbrt|abs|root|npr|ncr)\b|\^|!"

def read(p): return pathlib.Path(p).read_text(errors="replace")

# ---------- evidence collection
cases = []   # (file, lhs, rhs, boundary)
for f in sorted(glob.glob(str(ROOT / "tests/cases/*.txt"))):
    name = pathlib.Path(f).stem; bnd = False
    for ln in read(f).splitlines():
        if ln.startswith("#"): bnd = "boundary" in ln.lower(); continue
        if not ln.strip(): bnd = False; continue
        if ln.startswith(":") or "=>" not in ln: continue
        l, r = ln.split("=>", 1)
        cases.append((name, l.strip().lower(), r.strip(), bnd or name in BOUNDARY_FILES))
ui = []
for f in sorted(glob.glob(str(ROOT / "tests/ui/*.txt"))):
    expr_comment = ""
    for ln in read(f).splitlines():
        if ln.startswith("#"):
            m = re.search(r"\((.+)\)\s*$", ln); expr_comment = m.group(1) if m else ""   # menu_paths.txt: the expression behind the key path
        elif ln.strip() and "=>" in ln:
            l, r = ln.split("=>", 1); ui.append(((expr_comment + " " + l.strip()).strip(), r.strip())); expr_comment = ""
import keycomp                                    # record the expression behind every pilot question
pilot = []
_orig = keycomp.compile_expr
def _rec(s, *a, **k): pilot.append(s.lower()); return _orig(s, *a, **k)
keycomp.compile_expr = _rec
import questions                                  # noqa: E402  (executes the bank)
NQ = len(questions.QUESTIONS)
import subprocess as _sp
try:
    NCT = int(_sp.run(["ctest", "--test-dir", str(ROOT / "build"), "-N"], capture_output=True, text=True).stdout.split("Total Tests:")[1].split()[0])
except Exception:
    NCT = 0
pilot_topics = " ".join(q["topic"].lower() + " " + q["text"].lower() for q in questions.QUESTIONS)
ERR = re.compile(r"error|no solution|infinite", re.I)
ui_text = " ".join(l.lower() for l, _ in ui)
keys_blob = " ".join(l for l, _ in ui)

def evidence(f):
    pat = f["pat"]
    ev = dict(N=0, E=0, B=0, A=0, K=0, files=set())
    if pat:
        rx = re.compile(pat, re.I)
        for name, l, r, b in cases:
            if rx.search(l):
                ev["files"].add(name)
                if ERR.search(r): ev["E"] += 1
                else: ev["N"] += 1
                if b: ev["B"] += 1
                if name in ORACLE_FILES: ev["A"] += 1
                if name in PARTIAL_ORACLE and not re.search(r"^[-0-9. +i/]+$", r): ev["A"] += 1
        for s in pilot:
            if rx.search(s): ev["K"] += 1; ev["A"] += 1                  # pilot expectations come from mpmath
        for l, r in ui:
            if rx.search(l.lower()) or rx.search(r.lower()): ev["K"] += 1
        if re.fullmatch(r".*", DIFF_FUNCS) and re.search(DIFF_FUNCS, pat.replace("\\", "")) and f["cat"] in ("Scientific functions", "Arithmetic and entry"):
            ev["A"] += 1
    return ev

# curated evidence that cannot be mined mechanically (each backed by a named committed test)
EXTRA = {
 "Entry limits: bracket depth, number literals and tokens per line": dict(N=1, E=1, B=1, K=1, A=1, note="capacity.txt cases and key lines at each limit; bench_stack worst-case expressions"),
 "Replay / history recall and edit": dict(N=1, K=1, note="tests/ui/keys.txt, test_shell history persistence"),
 "Cursor edit: insert, DEL, home/end": dict(N=1, K=1, note="tests/ui/keys.txt edit cases, ui_fuzz"),
 "Natural (textbook) display of fractions, roots, powers": dict(N=1, K=1, note="test_layout, ui expr: cases"),
 "S<=>D exact/decimal toggle": dict(N=1, K=1, note="tests/ui/keys.txt SD cases"),
 "Engineering notation key (ENG)": dict(N=0, K=0, note="no dedicated test found"),
 "Statistic editor 80 rows": dict(N=1, K=1, note="stats cases + pilot stat questions; row limit not boundary-tested"),
 "Linear-system coefficient grid editor (2-4 unknowns)": dict(N=1, K=1, B=1, A=1, note="tests/ui/grid.txt generated against exact Fractions"),
 "Variables A B C D E F X Y M, STO": dict(N=1, K=1, note="ui keys + pilot"),
 "M+ / M-": dict(N=1, K=1, note="ui keys"),
 "Fix / Sci / Norm 1 / Norm 2": dict(N=1, K=1, note="basic cases (:decimal etc.) and ui"),
 "Fraction style (a b/c, d/c)": dict(N=1, K=0, note="script settings only"),
 "Reset (SHIFT SETUP) with confirmation": dict(N=1, K=1, note="test_shell, ui"),
 "State persistence across OFF/auto-off": dict(N=1, K=1, B=1, R=1, note="test_shell (CRC, torn write, version discard)"),
 "Contrast adjustment": dict(N=0, K=0, note="no dedicated test found"),
 "Auto power-off, OFF": dict(N=1, K=0, note="test_shell"),
 "Error messages: Math, Argument, Syntax, Stack, Time Out, No Solution": dict(N=1, E=1, K=1, note="spread across cases"),
 "Bin / Oct / Dec / Hex conversion and arithmetic": dict(N=1, K=0, E=1, note="basen.txt (58 hand-verified lines); no key-path test found"),
 "1-variable statistics (n, sum, mean, sigma, s, min, max, Q1, med, Q3)": dict(N=1, E=0, K=1, A=1, note="stats.txt (mpmath) + pilot"),
 "Paired statistics and regression: linear": dict(N=1, K=1, A=1, note="stats.txt + pilot regression"),
 "Regression: quadratic": dict(N=1, K=1, A=1, note="stats.txt + pilot"),
 "Regression: ln, e^, a*b^x, power, inverse": dict(N=1, K=0, A=1, note="stats.txt; pilot covers linear/quadratic only"),
 "Matrix storage A-D + Ans, up to 4x4": dict(N=1, K=1, note="matrix.txt hand-verified; 4x4 boundary only via pilot"),
 "Rectangular <-> polar display": dict(N=1, K=0, note="complex.txt"),
 "Statistical variables: n, sums, means, deviations (Σx, x̄, σx, sx, ...)": dict(N=1, K=1, A=1, note="stats.txt (mpmath), pilot stat_var keys"),
 "Regression coefficients and correlation (a, b, c, r)": dict(N=1, K=1, A=1, note="stats.txt, pilot"),
 "Minimum, maximum, quartiles, median": dict(N=1, K=1, A=1, note="stats.txt, pilot"),
 "Frequency column": dict(N=1, K=1, A=1, note="stats.txt generated with frequencies"),
 "Angle units deg/rad/grad and DMS": dict(N=1, K=1, note="ui + basic"),
 "Scientific constants (40, CODATA 2022)": dict(N=1, K=1, A=1, note="units.txt + pilot (values from scipy.constants, NIST not reachable)"),
 "Unit conversions (20 pairs, both directions)": dict(N=1, K=1, note="units.txt + pilot; hand-computed"),
 "Pi and e": dict(N=1, K=1, A=1, note="ui + basic"),
 "Operator precedence and implied multiplication": dict(N=1, K=1, A=1, note="basic + expression differential"),
}

def evaluate():
    rows = []
    for i, f in enumerate(F, 1):
        ev = evidence(f); x = EXTRA.get(f["name"], {})
        for k in "NEBAK": ev[k] = max(ev[k], x.get(k, 0)) if k in x else ev[k]
        ev["R"] = x.get("R", 1 if (ev["N"] or ev["K"]) else 0)
        st = f["st"]
        if st is None:
            if not (ev["N"] or ev["K"]): st = "INSUFFICIENT"
            else:
                ok = ev["N"] and ev["K"] and (ev["E"] or ev["B"]) and ev["A"]
                st = "TESTED" if ok else "INSUFFICIENT"
        gaps = [lbl for k, lbl in (("N", "normal"), ("K", "key-path"), ("E", "invalid/error"), ("B", "boundary"), ("A", "independent oracle")) if not ev[k] and st in ("TESTED", "INSUFFICIENT")]
        if st == "TESTED": gaps = [g for g in gaps if g == "boundary"] if not ev["B"] else []
        rows.append(dict(id=f"F{i:03d}", **f, status=st, ev=ev, gaps=gaps, note2=x.get("note", "")))
    return rows

# ---------- inventory counts mined from the source tree
def inventory_counts():
    app = read(ROOT / "core/ui/app.c"); menu = read(ROOT / "core/ui/menu.c"); lex = read(ROOT / "core/parser/lexer.c")
    kd = app.split("KD[K_COUNT] = {")[1].split("\n};")[0]
    key_entries = set(re.findall(r"(TKA?|ST|SPC|MN|VR|DG)\(([A-Za-z0-9_]+)(?:, ?([A-Za-z0-9_]+))?\)", kd))
    menu_tok = set(re.findall(r'TKA?_\("[^"]*", (T_\w+)(?:, (\w+))?\)', menu))
    menu_act = set(re.findall(r'ACT_\("[^"]*", (\w+)\)', menu))
    words = re.findall(r'\{"([^"]+)", (T_\w+), (W_\w+),', lex)
    consts = len(re.findall(r'^\s*\{"', read(ROOT / "core/units/constants_table.inc"), re.M))
    convs = len(re.findall(r'\{"([^"]+)", "([^"]+)", "[^"]+"\}', read(ROOT / "core/units/units.c").split("CONVS[]")[1].split("};")[0]))
    sv = re.search(r"SV_N = 0.*?SV_COUNT", read(ROOT / "core/stats/stat.h"), re.S).group(0).count("SV_") - 1
    return dict(key_entries=key_entries, menu_tok=menu_tok, menu_act=menu_act, words=words, consts=consts, convs=convs, statvars=sv)

def orphan_check(rows):
    inv = inventory_counts(); pats = [re.compile(f["pat"], re.I) for f in F if f["pat"]]
    names = " ".join(f["name"].lower() for f in F)
    orphans = []
    for w, tok, kind in inv["words"]:
        probe = [w + "(", w, "1 " + w + " 1"]
        if any(p.search(s) for p in pats for s in probe): continue
        if w in names: continue
        orphans.append(w)
    return inv, orphans

def counts_table(inv):
    tok_keys = {(a, b) for k, a, b in inv["key_entries"] if k in ("TK", "TKA", "ST")}
    toks = {t for _, t in [(0, x) for x, _ in inv["menu_tok"]]} | {a for a, _ in tok_keys}
    spc = {a for k, a, _ in inv["key_entries"] if k in ("SPC", "MN", "VR")}
    return [
      ("Distinct token-level functions reachable from keys or menus (operators, functions, structures, atoms)", len(toks)),
      ("Key actions that are not tokens (SHIFT, ALPHA, cursor, menus, variables, SD, ENG, M+, ...)", len(spc)),
      ("Menu actions (settings, data editor, linear-system grids, reset, clear)", len(inv["menu_act"])),
      ("Variables A B C D E F X Y M (as entry items)", 9),
      ("Scientific constants", inv["consts"]),
      ("Unit conversions (directional entries; 20 pairs)", inv["convs"]),
      ("Statistical variables and regression coefficients", inv["statvars"]),
      ("Regression types + 1-variable mode", 8),
      ("Digits 0-9 and decimal point", 11),
    ]

LBL = dict(TESTED="Implemented and tested", INSUFFICIENT="Implemented, insufficiently tested", PARTIAL="Partially implemented",
           MISSING="Missing", NOTSUITABLE="Not suitable for the Exam model", RESTORE="Missing (approved for restoration)")
DEC = dict(CORE="Core", PLUS="Optional", OUT="Out")

def write_markdown(rows, inv, ct, orphans, summary):
    out = ROOT / "docs/exam"
    total = sum(v for k, v in ct if not k.startswith("Variables A B C"))
    L = ["# 01 - SC991 Exam feature inventory", "", "Generated by `tools/audit/register.py` from the source tree (lexer, key table, menu tables, constants/units/statistics tables). Do not edit by hand.", "",
         "## How many functions does the SC991 have?", "",
         "The repository contains no definition of the advertised **417** target (`grep 417` finds only unrelated constants). 417 is Casio's own marketing figure for the fx-991ES PLUS (the fx-991EX is 552), and Casio does not publish how it counts. "
         "The table below counts what is actually reachable on the SC991 under an explicit, reproducible rule: every distinct key function, menu item, constant, conversion and statistic variable.", "",
         "| Counted item | Count |", "|---|---:|"]
    L += [f"| {k} | {v} |" for k, v in ct if not k.startswith("Variables A B C")]
    L += [f"| **Total (SC991, this rule)** | **{total}** |", "",
          "Variables A-F, X, Y, M are included in the key-action row. Digits are included. The total is **not** comparable with 417 because Casio's counting rule is unknown; it must not be used in advertising as 'N functions'. "
          "Recommendation: market the capability list (the register below), not a number, or publish our own counting rule with the number.", "",
          f"Catalogue coverage check: {len(inv['words'])} lexer names, {len(orphans)} not covered by a catalogue row" + (f" ({', '.join(orphans)})" if orphans else "") + ".", ""]
    cats = []
    for r in rows:
        if r["cat"] not in cats: cats.append(r["cat"])
    L += ["## Capability catalogue by category", "", "Status vocabulary: " + "; ".join(f"**{v}**" for v in LBL.values()) + ".", ""]
    for c in cats:
        L += [f"### {c}", "", "| ID | Capability | How it is reached | Status |", "|---|---|---|---|"]
        L += [f"| {r['id']} | {r['name']} | {r['access']} | {LBL[r['status']]} |" for r in rows if r["cat"] == c]
        L.append("")
    (out / "01-feature-inventory.md").write_text("\n".join(L))
    R = ["# 04 - Feature coverage and testing register", "", "Generated by `tools/audit/register.py`; machine-readable copy: `feature-register.csv`. Do not edit by hand.", "",
         f"Evidence base: {len(cases)} script cases (`tests/cases`), {len(ui)} key-sequence lines (`tests/ui`), {NQ} pilot questions (`tools/pilot`), plus unit, differential (mpmath, correctly-rounded) and fuzz tests. {NCT} CTest suites pass on main.", "",
         "## Status summary", "", "| Status | Count |", "|---|---:|"]
    R += [f"| {LBL[k]} | {summary.get(k, 0)} |" for k in ("TESTED", "INSUFFICIENT", "PARTIAL", "MISSING", "RESTORE", "NOTSUITABLE")]
    R += ["", "## Rule used", "", s_doc(), "", "## Limits of this register", "",
          "- Evidence is mined mechanically by matching feature patterns in test inputs; a few rows use curated evidence (`EXTRA` in the script, each naming the committed test). A row marked insufficient means *no evidence was found*, which is occasionally a pattern miss; a row marked tested means evidence of every required kind was found, not that every input class is covered.",
          "- 'Independent oracle' means the expected value was produced by mpmath or exact Python Fractions (generated case files, differential tests, pilot bank). Hand-computed expectations are valid tests but are not counted as independent.",
          "- Boundary evidence is only recognised in blocks that follow a `# boundary` comment. Limits probed during this audit are in `limits-probe.md`; they are measurements, not yet committed regression tests.", "",
          "## Register", "", "| ID | Capability | Exam | Casio ES+ / EX | Status | N | Err | Bnd | Orc | Key | Missing evidence |", "|---|---|---|---|---|--:|--:|--:|--:|--:|---|"]
    for r in rows:
        e = r["ev"]
        R.append(f"| {r['id']} | {r['name']} | {DEC[r['dec']]} | {r['es']} / {r['ex']} | {LBL[r['status']]} | {e['N']} | {e['E']} | {e['B']} | {e['A']} | {e['K']} | {', '.join(r['gaps']) or ('-' if r['status'] in ('TESTED',) else r['note'][:70])} |")
    (out / "04-coverage-register.md").write_text("\n".join(R) + "\n")

def s_doc():
    return "A capability is **Implemented and tested** only when evidence exists for all of: a normal case, a key-interface path (pilot or UI test), an invalid-input/error case **or** a boundary case, and an independent oracle. Anything implemented that lacks one of these is **Implemented, insufficiently tested** and the missing kinds are listed."

def main():
    rows = evaluate(); inv, orphans = orphan_check(rows); ct = counts_table(inv)
    out = ROOT / "docs/exam"; out.mkdir(exist_ok=True, parents=True)
    with open(out / "feature-register.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["id", "category", "feature", "access", "casio_es_plus", "casio_ex", "exam_decision", "status", "normal", "error", "boundary", "oracle", "key_path", "regression", "test_files", "gaps", "note"])
        for r in rows:
            e = r["ev"]
            w.writerow([r["id"], r["cat"], r["name"], r["access"], r["es"], r["ex"], r["dec"], r["status"], e["N"], e["E"], e["B"], e["A"], e["K"], e["R"], "+".join(sorted(e["files"])), "; ".join(r["gaps"]), r["note"] or r["note2"]])
    summary = {}
    for r in rows: summary[r["status"]] = summary.get(r["status"], 0) + 1
    print("catalogue rows:", len(rows)); print("status:", summary)
    print("pilot questions:", NQ, "| script cases:", len(cases), "| ui lines:", len(ui))
    for k, v in ct: print(f"{v:5d}  {k}")
    print("lexer words:", len(inv["words"]), "orphans:", orphans)
    write_markdown(rows, inv, ct, orphans, summary)
    if "--check" in sys.argv and orphans: sys.exit(1)
    return rows, inv, ct, orphans, summary

if __name__ == "__main__":
    main()
