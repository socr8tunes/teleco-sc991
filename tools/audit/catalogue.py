"""Curated SC991 Exam feature catalogue.  One row per user-visible function/capability.

Columns: category, name, access path, test pattern (regex over expression text, or None),
         Casio fx-991ES PLUS (Y/N/?), Casio fx-991EX (Y/N/?), Exam decision, status override, note.
Exam decision: CORE (credible fx-991-class must have), PLUS (valuable, optional), OUT (not for the Exam model).
Status override: None = derive from test evidence; otherwise PARTIAL / MISSING / NOTSUITABLE / RESTORE.
Reference columns: Y = confirmed on Casio's pages/quick-start guide fetched 2026-11-02 (see 02-casio-gap-analysis.md);
? = from general knowledge, must be checked against the official user guides before any marketing claim.
"""
F = []
def a(cat, name, access, pat=None, es="Y", ex="Y", dec="CORE", st=None, note=""):
    F.append(dict(cat=cat, name=name, access=access, pat=pat, es=es, ex=ex, dec=dec, st=st, note=note))

# ---- arithmetic and entry
C = "Arithmetic and entry"
a(C, "Four operations + - x /", "+ - x /", r"[0-9)]\s*[-+*/]\s*[0-9(]")
a(C, "Operator precedence and implied multiplication", "entry", r"2\*3\^2|1/2pi|2pi")
a(C, "Unary minus (-)", "(-)", r"\(-|^-")
a(C, "Brackets", "( )", r"\(")
a(C, "Entry limits: bracket depth, number literals and tokens per line", "entry", r"stack error", st="PARTIAL",
  note="measured: 11 bracket levels (docs claim 24), 40 number literals per expression; see limits-probe.md")
a(C, "Power x^n (any expression exponent)", "x^n", r"\^")
a(C, "Square x^2 / cube (SHIFT)", "x^2", r"\^2|sq")
a(C, "Reciprocal x^-1", "x^-1", r"\^-1|recip")
a(C, "Square root", "sqrt", r"sqrt")
a(C, "n-th root", "SHIFT sqrt", r"root")
a(C, "Cube root", "MENU>Number", r"cbrt")
a(C, "Factorial x!", "x!", r"!")
a(C, "Percent %", "SHIFT (", r"%")
a(C, "EXP entry x10^x", "x10^x", r"[0-9]E[0-9-]")
a(C, "Ans / PreAns", "Ans", r"\bans\b|preans")
a(C, "Replay / history recall and edit", "up/down, left/right", None)
a(C, "Cursor edit: insert, DEL, home/end", "keys", None)
a(C, "Natural (textbook) display of fractions, roots, powers", "a/b, sqrt, x^n", None)
a(C, "Stacked fraction / mixed fraction entry", "a/b, SHIFT a/b", r"frac")
a(C, "S<=>D exact/decimal toggle", "S<>D", None)
a(C, "Exact results: fractions, surds, multiples of pi", "result", r"sqrt|pi|frac")
a(C, "Engineering notation key (ENG)", "ENG", None)
a(C, "Engineering symbols in input (k, M, m, n ...)", "SETUP", None, es="?", ex="Y", dec="PLUS", st="MISSING")
a(C, "Digit separator display", "SETUP", None, es="?", ex="Y", dec="PLUS", st="MISSING")
a(C, "Multi-statement entry (':')", "ALPHA =", None, dec="OUT", st="NOTSUITABLE",
  note="removed by product decision; the Exam model keeps one expression per entry. Revisit only with WAEC input")
a(C, "UNDO", "-", None, es="N", ex="?", dec="PLUS", st="MISSING", note="Casio ES PLUS has no UNDO; one-level undo of the last edit is a usability option")
# ---- scientific functions
C = "Scientific functions"
for n, p in [("sin", "sin"), ("cos", "cos"), ("tan", "tan"), ("sin^-1", "asin"), ("cos^-1", "acos"), ("tan^-1", "atan")]:
    a(C, n, "key" if len(p) == 3 else "SHIFT key", r"(?<![a-z])" + p + r"\(")
for n, p in [("sinh", "sinh"), ("cosh", "cosh"), ("tanh", "tanh"), ("sinh^-1", "asinh"), ("cosh^-1", "acosh"), ("tanh^-1", "atanh")]:
    a(C, n, "MENU>Hyperbolic / hyp", r"(?<![a-z])" + p + r"\(")
a(C, "log10", "log", r"(?<![a-z])log\(")
a(C, "ln", "ln", r"(?<![a-z])ln\(")
a(C, "log base b", "MENU>Number", r"logb|log\([^,)]+,")
a(C, "e^x", "SHIFT ln", r"(?<![a-z])exp\(")
a(C, "10^x", "SHIFT log", r"alog|10\^")
a(C, "Angle units deg/rad/grad and DMS", "SETUP, DMS", r"\bdeg\b|\brad\b|grad|dms")
a(C, "DMS entry and DMS result", "DMS, SHIFT DMS", r"dms")
a(C, "Abs", "MENU>Number", r"(?<![a-z])abs\(")
a(C, "Int / Intg", "MENU>Number", r"(?<![a-z])intg?\(")
a(C, "Rnd", "MENU>Number", r"(?<![a-z])rnd\(")
a(C, "Pol(", "SHIFT +", r"pol\(")
a(C, "Rec(", "SHIFT -", r"rec\(")
a(C, "Ran#", "SHIFT .", r"ran#")
a(C, "RanInt(", "SHIFT 0", r"ranint\(")
a(C, "nPr", "SHIFT x", r"npr")
a(C, "nCr", "SHIFT x", r"ncr")
a(C, "Pi and e", "pi, SHIFT pi", r"\bpi\b")
a(C, "Scientific constants (40, CODATA 2022)", "SHIFT 7", r"const\(")
a(C, "Unit conversions (20 pairs, both directions)", "SHIFT 8", r"conv\(")
# ---- number theory
C = "Number theory"
a(C, "Prime factorisation", "MENU>Number>PFact", r"pfact\(", dec="CORE")
a(C, "GCD", "MENU>Number", r"(?<![a-z])gcd\(", es="?", ex="?", dec="PLUS")
a(C, "LCM", "MENU>Number", r"(?<![a-z])lcm\(", es="?", ex="?", dec="PLUS")
a(C, "Integer quotient Quot", "MENU>Number", r"quot\(", es="?", ex="?", dec="PLUS")
a(C, "Remainder Rem", "MENU>Number", r"(?<![a-z])rem\(", es="?", ex="?", dec="PLUS")
# ---- calculus
C = "Calculus and sums"
a(C, "Summation", "MENU>Calculus", r"(?<![a-z])sum\(")
a(C, "Product operator", "MENU>Calculus", r"prod\(", es="N", ex="Y", dec="PLUS")
a(C, "Derivative d/dx", "MENU>Calculus", r"ddx\(")
a(C, "Definite integral", "MENU>Calculus", r"integral\(")
a(C, "Solve (Newton)", "MENU>Calculus", r"solve\(")
a(C, "CALC (re-evaluate with variable prompts)", "CALC key", None, ex="?", dec="CORE", st="RESTORE",
  note="approved for restoration as a session-only, single-expression tool; not yet built")
a(C, "TABLE f(x) (one function)", "TABLE", None, dec="CORE", st="RESTORE",
  note="approved for restoration; fx-991EX offers f(x) and g(x), the Exam model uses one f(X)")
# ---- equations
C = "Equations"
a(C, "Quadratic", "MENU>Equations", r"quad\(")
a(C, "Cubic", "MENU>Equations", r"cubic\(")
a(C, "Quartic (degree 4)", "MENU>Equations", None, es="N", ex="Y", dec="CORE", st="PARTIAL",
  note="engine written, parked on branch wip/quartic-parked, untested, not on main")
a(C, "Simultaneous equations, 2 unknowns", "MENU>Equations", r"simul2\(")
a(C, "Simultaneous equations, 3 unknowns", "MENU>Equations", r"simul3\(")
a(C, "Simultaneous equations, 4 unknowns", "MENU>Equations", r"simul4\(", es="N", ex="Y", dec="CORE")
a(C, "Linear-system coefficient grid editor (2-4 unknowns)", "MENU>Equations", None)
a(C, "Inequality solver, degree 2-4", "-", None, es="N", ex="Y", dec="CORE", st="MISSING")
a(C, "Ratio solver A:B=X:D", "-", None, es="N", ex="Y", dec="PLUS", st="MISSING")
# ---- complex
C = "Complex numbers"
for n, p in [("i (imaginary unit)", r"\bi\b"), ("Arg", r"arg\("), ("Conjg", r"conj\("), ("Re", r"(?<![a-z])re\("), ("Im", r"(?<![a-z])im\(")]:
    a(C, n, "MENU>Complex", p)
a(C, "Complex arithmetic, powers, roots", "SETUP complex on", r"\bi\b")
a(C, "Rectangular <-> polar display", "SETUP", r"cmplx|polar")
a(C, "Complex asin/acos/atan, 10^z", "-", None, es="?", ex="?", dec="PLUS", st="PARTIAL", note="not supported; e^ and ^ work")
# ---- matrices, vectors
C = "Matrix and vector"
a(C, "Matrix storage A-D + Ans, up to 4x4", "MENU>Matrix", r"\[|mat[a-d]|matans", es="Y", ex="Y",
  note="ES PLUS: 3 matrices 3x3 (from memory); EX: 4x4 (confirmed by QSG)")
a(C, "Matrix + - x, scalar", "expression", r"\[.*\]\s*[*+-]")
a(C, "Det", "MENU>Matrix", r"det\(")
a(C, "Trn", "MENU>Matrix", r"trn\(")
a(C, "Iden", "MENU>Matrix", r"iden\(")
a(C, "Matrix inverse x^-1 and integer powers", "x^-1", r"\]\^-1|\]\^2")
a(C, "Vector storage A-D + Ans, 2/3 dimensions", "MENU>Vector", r"vct")
a(C, "Dot", "MENU>Vector", r"dot\(")
a(C, "Cross", "MENU>Vector", r"cross\(")
a(C, "Angle between vectors", "MENU>Vector", r"anglev\(")
a(C, "Unit vector", "MENU>Vector", r"unitv\(")
a(C, "Abs of vector", "MENU>Vector", r"abs\(vct")
# ---- statistics
C = "Statistics"
a(C, "1-variable statistics (n, sum, mean, sigma, s, min, max, Q1, med, Q3)", "MENU>Statistics", r"1var", None)
a(C, "Paired statistics and regression: linear", "SETUP/MENU", r"stat linear|reg_a", None)
a(C, "Regression: quadratic", "SETUP", r"quad", None)
a(C, "Regression: ln, e^, a*b^x, power, inverse", "SETUP", r"stat (ln|exp|ab|pow|inv)", None)
a(C, "Estimates y^ and x^", "MENU>Statistics", r"yhat|xhat")
a(C, "Statistical variables: n, sums, means, deviations (Σx, x̄, σx, sx, ...)", "MENU>Statistics>Variables", r"(?<![a-z_0-9])(n|sumx2?|xbar|sigx|sx|sumy2?|ybar|sigy|sy|sumxy|sumx3|sumx4|sumx2y)(?![a-z_0-9(])", None)
a(C, "Regression coefficients and correlation (a, b, c, r)", "MENU>Statistics>Variables", r"reg_[abcr]", None)
a(C, "Minimum, maximum, quartiles, median", "MENU>Statistics>Variables", r"(?<![a-z_0-9])(minx|maxx|miny|maxy|q1|med|q3)(?![a-z_0-9(])", None)
a(C, "Frequency column", "editor", None)
a(C, "Statistic editor 80 rows", "SHIFT MENU", None, note="ES/EX 1-var 80 rows; paired 40 on ES (from memory)")
# ---- distributions
C = "Distributions"
a(C, "Normal P(t), Q(t), R(t)", "MENU>Distribution", r"norm[pqr]\(")
a(C, "Normal density PD and general normal CD (lower, upper, sigma, mu)", "-", None, es="?", ex="Y", dec="PLUS", st="MISSING",
  note="EX Distribution mode; SC991 has standard-normal P/Q/R and InvN(area, sigma, mu) only")
a(C, "Inverse normal", "MENU>Distribution", r"invn\(", es="N", ex="Y")
a(C, "Binomial PD / CD", "MENU>Distribution", r"binom[pc]d\(", es="N", ex="Y")
a(C, "Poisson PD / CD", "MENU>Distribution", r"pois[pc]d\(", es="N", ex="Y")
# ---- base-n, logic
C = "Base-n and logic"
a(C, "Bin / Oct / Dec / Hex conversion and arithmetic", "SETUP", None)
a(C, "and, or, xor, xnor", "MENU>Prob", r"\b(and|or|xor|xnor)\b")
a(C, "Not, Neg", "MENU>Prob", r"not\(|neg\(")
# ---- memory, setup
C = "Memory, display and settings"
a(C, "Variables A B C D E F X Y M, STO", "STO, ALPHA", None)
a(C, "M+ / M-", "M+, SHIFT M+", None)
a(C, "Fix / Sci / Norm 1 / Norm 2", "SETUP", None)
a(C, "Fraction style (a b/c, d/c)", "SETUP", None)
a(C, "Reset (SHIFT SETUP) with confirmation", "SHIFT SETUP", None)
a(C, "State persistence across OFF/auto-off", "-", None, dec="PLUS", note="power-fail safe commit, CRC; incompatible blobs discarded on format change")
a(C, "Contrast adjustment", "SHIFT up/down", None)
a(C, "Auto power-off, OFF", "SHIFT AC", None)
a(C, "Error messages: Math, Argument, Syntax, Stack, Time Out, No Solution", "-", None)
# ---- out of scope
C = "Casio capabilities not for the Exam model"
a(C, "Spreadsheet mode", "-", None, es="N", ex="Y", dec="OUT", st="NOTSUITABLE", note="excluded by product decision (user-written formulas)")
a(C, "QR code / online services", "-", None, es="N", ex="Y", dec="OUT", st="NOTSUITABLE", note="needs connectivity; violates self-contained rule")
a(C, "Programming, macros, scripting", "-", None, es="N", ex="N", dec="OUT", st="NOTSUITABLE", note="SC991 Pro only")
a(C, "Graphing", "-", None, es="N", ex="N", dec="OUT", st="NOTSUITABLE", note="non-graphing product")
a(C, "AI / natural-language features", "-", None, es="N", ex="N", dec="OUT", st="NOTSUITABLE", note="SC991 AI only")
a(C, "CAS / symbolic algebra", "-", None, es="N", ex="N", dec="OUT", st="NOTSUITABLE")
