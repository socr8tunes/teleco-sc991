"""Compile textbook notation into SC991 key scripts (what a student would actually press).

  sqrt{x}  frac{a}{b}  root{n}{x}  ^{..}  ^2  ^3  ^-1  !  %  sin( cos( tan( asin( acos( atan( log( ln( exp( alog(
  pi e Ans  A..F X Y M (variables)  nCr nPr  ->A (store)  [[RAW KEYS]]  numbers with E exponents
A '-' at the start, after an operator or after '(' is the negative sign key, otherwise subtraction.
"""
import re

VARKEY = {"A": "NEG", "B": "DMS", "C": "HYP", "D": "SIN", "E": "COS", "F": "TAN", "X": ")", "Y": "(", "M": "M+"}
FUNC = {"sin": "SIN", "cos": "COS", "tan": "TAN", "asin": "SHIFT SIN", "acos": "SHIFT COS", "atan": "SHIFT TAN",
        "log": "LOG", "ln": "LN", "pol": "SHIFT +", "rec": "SHIFT -", "ranint": "SHIFT 0", "exp": "SHIFT LN", "alog": "SHIFT LOG"}
MENU_FN = {  # name -> (menu number, item number) in the MENU catalogue (see core/ui/menu.c)
    "sinh": (1, 1), "cosh": (1, 2), "tanh": (1, 3), "asinh": (1, 4), "acosh": (1, 5), "atanh": (1, 6),
    "sum": (2, 1), "prod": (2, 7), "ddx": (2, 2), "integral": (2, 3), "solve": (2, 4),
    "abs": (3, 1), "int": (3, 2), "intg": (3, 3), "rnd": (3, 4), "cbrt": (3, 5), "arg": (5, 2), "conj": (5, 3), "re": (5, 4), "im": (5, 5),
    "det": (6, 10), "trn": (6, 11), "iden": (6, 12),
    "gcd": (3, 13), "lcm": (3, 14), "quot": (3, 15), "rem": (3, 16), "pfact": (3, 17),
    "normp": (9, 1), "normq": (9, 2), "normr": (9, 3), "invn": (9, 4), "binompd": (9, 5), "binomcd": (9, 6), "poispd": (9, 7), "poiscd": (9, 8),
    "quad": (10, 1), "cubic": (10, 2), "simul2": (10, 3), "simul3": (10, 4),
}


def menu_pick(menu, item):
    """Keys that open main-menu entry `menu` and choose its `item` (1-based)."""
    first = f"MENU {menu}" if menu <= 9 else "MENU " + " ".join(["DOWN"] * (menu - 1)) + " EXE"
    if item <= 9:
        return f"{first} {item}"
    return f"{first} " + " ".join(["DOWN"] * (item - 1)) + " EXE"


def compile_expr(s):
    out = []
    i = 0
    n = len(s)
    prev = "start"   # start | op | open | value

    def emit(k, state):
        nonlocal prev
        out.append(k)
        prev = state

    def group():
        """read a {...} group, return its inner text"""
        nonlocal i
        assert s[i] == "{", s[i:]
        depth, j = 0, i
        while True:
            if s[j] == "{": depth += 1
            elif s[j] == "}":
                depth -= 1
                if depth == 0: break
            j += 1
        inner = s[i + 1:j]
        i = j + 1
        return inner

    while i < n:
        c = s[i]
        if c == " ":
            i += 1
            continue
        if s.startswith("[[", i):
            j = s.index("]]", i)
            emit(s[i + 2:j].strip(), "value"); i = j + 2
            continue
        m = re.match(r"\d+(\.\d*)?|\.\d+", s[i:])
        if m:
            for ch in m.group(0):
                out.append("." if ch == "." else ch)
            i += len(m.group(0)); prev = "value"
            m2 = re.match(r"E(-?)(\d+)", s[i:])
            if m2:
                out.append("EE")
                if m2.group(1): out.append("NEG")
                out.extend(m2.group(2))
                i += m2.end()
            continue
        if s.startswith("sqrt{", i):
            i += 4; inner = group()
            emit("SQRT " + compile_expr(inner) + " RIGHT", "value"); continue
        if s.startswith("frac{", i):
            i += 4; a = group(); b = group()
            emit("FRAC " + compile_expr(a) + " DOWN " + compile_expr(b) + " RIGHT", "value"); continue
        if s.startswith("root{", i):
            i += 4; a = group(); b = group()
            emit("SHIFT SQRT " + compile_expr(a) + " RIGHT " + compile_expr(b) + " RIGHT", "value"); continue
        if s.startswith("^{", i):
            i += 1; inner = group()
            emit("POW " + compile_expr(inner) + " RIGHT", "value"); continue
        mp_ = re.match(r"\^(-?\d+(?:\.\d+)?)", s[i:])
        if mp_:
            e_ = mp_.group(1)
            if e_ == "2": emit("SQ", "value")
            elif e_ == "3": emit("SHIFT SQ", "value")
            elif e_ == "-1": emit("XINV", "value")
            else: emit("POW " + compile_expr(e_) + " RIGHT", "value")
            i += mp_.end(); continue
        if s.startswith("->", i):
            v = s[i + 2]
            emit("STO ALPHA " + VARKEY[v], "value"); i += 3; continue
        if c == "!": emit("FACT", "value"); i += 1; continue
        if c == "%": emit("SHIFT (", "value"); i += 1; continue
        if c == "+": emit("+", "op"); i += 1; continue
        if c == "-":
            emit("NEG" if prev in ("start", "op", "open") else "-", "op"); i += 1; continue
        if c == "*": emit("*", "op"); i += 1; continue
        if c == "/": emit("/", "op"); i += 1; continue
        if c == "(": emit("(", "open"); i += 1; continue
        if c == ")": emit(")", "value"); i += 1; continue
        if c == ",": emit("SHIFT )", "op"); i += 1; continue
        if c == "=": emit("SHIFT EXE", "op"); i += 1; continue
        if s.startswith("nCr", i): emit("SHIFT /", "op"); i += 3; continue
        if s.startswith("nPr", i): emit("SHIFT *", "op"); i += 3; continue
        if s.startswith("Ans", i): emit("ANS", "value"); i += 3; continue
        if s.startswith("pi", i): emit("PI", "value"); i += 2; continue
        m = re.match(r"[a-z][a-z0-9]*\(", s[i:])
        if m:
            name = m.group(0)[:-1]
            if name in FUNC: emit(FUNC[name], "open")
            elif MENU_FN.get(name): emit(menu_pick(*MENU_FN[name]), "open")
            else: raise ValueError("unknown function " + name)
            i += len(m.group(0)); continue
        if c == "e" and not s[i:i + 2].isalpha() or (c == "e" and not s[i + 1:i + 2].isalpha()):
            emit("SHIFT PI", "value"); i += 1; continue
        if c == "i" and not s[i + 1:i + 2].isalpha():
            emit(menu_pick(5, 1), "value"); i += 1; continue
        if c in VARKEY:
            emit("ALPHA " + VARKEY[c], "value"); i += 1; continue
        raise ValueError(f"cannot compile {s[i:]!r} in {s!r}")
    return " ".join(x for x in out if x)


def stat_keys(stat_type, xs, ys=None, freqs=None):
    """Enter statistics data from the STAT editor. stat_type 1 = 1-var, 2 = y=a+bx, 3 = quadratic ... (SETUP menu order)."""
    k = [f"MODE 7 {stat_type}"]
    num = lambda v: ("NEG " + str(v)[1:]) if str(v).startswith("-") else str(v)
    for x in xs:
        k.append(num(x) + " EXE")
    if freqs:
        k.append("UP " * len(xs))
        k.append("RIGHT" + ("" if ys is None else " RIGHT"))
        for f in freqs:
            k.append(f"{f} EXE")
    if ys is not None:
        k.append("UP " * len(xs))
        k.append("RIGHT")
        for y in ys:
            k.append(num(y) + " EXE")
    return " ".join(x for x in k if x.strip())


def stat_var(var_id):
    """Keys that leave the editor, open MENU > Statistics > Variables, insert variable #var_id and press =."""
    return "AC MENU 8 3 " + " ".join(["DOWN"] * var_id) + " EXE EXE"
