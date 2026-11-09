# 03 - Proposed final SC991 Exam feature specification (revised 2026-11-07, DRAFT for approval)

Product: SC991 Exam - non-programmable, non-graphing, silent, cordless, self-contained scientific calculator for Ghanaian SHS and university students and general users. Benchmark: Casio fx-991ES PLUS / fx-991EX capabilities (not their branding or interface). Subject to WAEC review; **no approval or compliance is claimed, and none may be claimed without written confirmation from WAEC**.

## 1. Product boundaries (hard rules, enforced by tests)

1. No user programs, macros, scripts, user-defined functions or persistent executable formulas.
2. No graphing or plotting. TABLE shows numbers only.
3. No spreadsheet, no external data import/export, no QR, no radio/IO of any kind in firmware. (Firmware programming port is a manufacturing feature, disabled or locked in shipped units - hardware decision, see risks.)
4. No symbolic algebra: results are numeric, exact rational, surd, pi-multiple or complex-rectangular/polar only. No expression simplification, no factoring of user expressions, no differentiation or integration to a symbolic form.
5. No AI. No multi-statement chaining. Session-only helpers (CALC, TABLE) keep exactly one expression and are cleared by AC, leaving the mode and power-off; never written to non-volatile storage.
6. Guard test (**implemented**, ctest `exclusions`, `tests/unit/test_exclusions.c`): the set of token labels must equal the reviewed list `tests/exam/approved_tokens.txt` (adding any token fails the test until the list is edited on purpose); every item of every menu is visited by key presses and may insert only approved tokens; no menu label may name a forbidden capability (program, macro, script, sheet, graph, plot, table while on hold, wireless, import/export, AI ...); `1:2` and `table(`, `calc(`, `prog(`, `spreadsheet(` are Syntax ERROR (`tests/cases/basic.txt`). Still to write when CALC/TABLE are implemented: a save/load test that their content is absent from the persisted blob (the blob today is header + settings/memory + replay history + CRC only).

## 2. Functional scope

**Core (must ship for fx-991-class credibility)**

| Area | Scope |
|---|---|
| Calculate | + - x /, brackets, implied multiplication, exact fractions, mixed numbers, surds, multiples of pi, S<=>D, %, Ans/PreAns, replay with edit, 9 variables, M+/M- |
| Functions | trig and inverse (D/R/G), hyperbolic and inverse, log, ln, log base b, e^x, 10^x, x^2, x^3, x^n, roots, cube root, reciprocal, factorial, nPr, nCr, Pol/Rec, Abs, Int, Intg, Rnd, Ran#, RanInt, DMS in/out |
| Calculus | Sigma, derivative at a point, definite integral, Solve (Newton with safeguards) |
| CALC | one expression on the entry line; prompts for each variable used (A-F, X, Y, M); re-evaluate with new values; session-only (approved 2026-11-02) |
| TABLE | one f(X); start, end, step; at most 30 rows; any ordinary math function allowed inside f; view-only; session-only (approved) |
| Equations | simultaneous 2, 3, 4 unknowns (unique / inconsistent / underdetermined handled, exact when possible); polynomial degree 2, 3, 4 with exact roots where practical, multiplicity preserved, complex roots |
| Inequality | degree 2-4; one interval per page with previous/next, "k/N" indicator, explicit endpoint inclusion; special results (all reals, no solution) (approved) |
| Complex | rectangular and polar, Arg, Conjg, Re, Im, powers, roots |
| Matrix | 4 stored matrices + Ans, up to 4x4: + - x scalar, Det, Trn, Iden, inverse, integer powers |
| Vector | 4 stored vectors + Ans, 2-3 dimensions: + - x scalar, Dot, Cross, Angle, UnitV, Abs |
| Statistics | 1-variable and paired, frequency column, 80 rows, 7 regression types, estimates, all summary statistics |
| Distributions | standard normal P/Q/R, inverse normal, binomial PD/CD, Poisson PD/CD |
| Base-n | Bin/Oct/Dec/Hex arithmetic and conversion, and/or/xor/xnor/Not/Neg |
| Number | prime factorisation, GCD, LCM, quotient, remainder, product operator |
| Reference | 40 constants, 20 conversion pairs, DMS |

**Optional (build after Core is fully tested; each independently droppable)**: Ratio solver; Normal PD and general Normal CD(lower, upper, sigma, mu); engineering symbols and digit separator; one-level UNDO of the last edit; complex inverse-trig and 10^z; list output for binomial/Poisson.

**Out for the Exam model**: spreadsheet, QR, multi-statement, programming, graphing, AI, CAS.

## 3. Numerical and capacity requirements

Targets set by the product owner on 2026-11-05: **at least 24 usable nested bracket levels and 80 number literals per expression**. Both are implemented and boundary-tested.

### 3.1 Entry-capacity table (implemented and tested)

| Item | Limit | Behaviour at the limit | Evidence |
|---|---|---|---|
| Nested levels (brackets, function openers, fractions, roots, powers, mixed numbers) | **24** usable (`MAX_NEST`); separate internal recursion guard 120 | Level 24 evaluates. Level 25 gives **Stack ERROR** on EXE; the line stays editable (DEL back to 24 and EXE again, tested) | `cases/capacity.txt`, `ui/capacity.txt`, `bench_stack` |
| Number literals per expression | **80** (`MAX_CONSTS`) | 80th evaluates; 81st gives **Stack ERROR** on EXE | same |
| Tokens (keystrokes) per entry line | **256** (`ED_CAP`) | Further keys are **ignored** (no error, no change); content intact | `ui/capacity.txt` |
| Total expression length | 256 tokens as typed. A digit, operator, variable or function opener (`sin(`) is 1 token; a fraction, root, power, mixed number is a begin and an end token (2+); `0.5` is 3 tokens, so the token limit is reached before 80 literals when literals are not integers | as above | same |
| Token capacity after auto-closing brackets | 288 (`MAX_TOKENS`) = 256 + 32 closers | cannot be exceeded from the editor | design |
| Compiled instructions | 384 (`MAX_CODE`) | **Stack ERROR** on EXE | `capacity.txt` |
| Evaluation stack | 48 values (`EVAL_STACK`) | **Stack ERROR** | `bench_stack` |
| Nested Sigma / Pi / integral / d/dx / Solve bodies at run time | 23 inside one another (`EVAL_DEPTH` 24 counts the top level; 23 nested sums evaluate, 24 give Stack ERROR) | **Stack ERROR** | `capacity.txt` |
| Sigma / Pi terms | 100001 (hi - lo <= 100000) | **Time Out** beyond; non-integer bounds Argument ERROR | `oracle_calculus.txt` |
| VM work budget per EXE | 4,000,000 steps | **Time Out** | design (to be re-set by time on hardware, risk R4) |
| Replay history | 480 tokens in total; records over 255 tokens are executed but not stored | oldest entries are dropped | `test_shell`, `app.c` guard |
| Statistics rows | 80 in every mode (Casio ES PLUS 80 / 40 paired / 26 with frequency; EX 160 / 80 / 53) | 81st row: Range ERROR | `limits-probe.md` |
| Matrix / vector | 4x4 (4 stored + Ans) / 2-3 dimensions | 5x5: Dimension ERROR; 4-vector: Argument ERROR | `limits-probe.md` |
| Casio EX comparison | EX accepts 159 bytes of entry; the SC991 accepts 256 tokens (a byte-per-key comparison depends on token weights and is only indicative) | - | Casio guide |

### 3.2 Other numerical requirements

| Item | Value |
|---|---|
| Internal arithmetic | 16-digit decimal, exponent +-99 (results below 1E-99 flush to 0; results at or above 1E100 are Math ERROR), exact rational/surd tracking, 45-54 digit extended path |
| Display | 10 significant digits (Norm), Fix/Sci/Eng |
| Accuracy | functions correctly rounded vs mpmath (differential tests); expression differential; ~2,800 independent-oracle cases (matrix, vector, complex, base-n, domain, statistics errors, calculus, equations, scientific edges) |
| Calculus accuracy | numerical methods, about 9-10 digits typical; singular integrals and extreme-scale arguments can lose digits (known defects below) |
| Complex roots order | a conjugate pair is always listed with the positive imaginary part first |
| Run-time | not measured on target; each CORE operation must return within 2 s on the chosen MCU; sums, products, integrals, Solve bounded by Time Out |

### 3.3 Numerical methods: behaviour rules (implemented in P1, decisions D4 and D5)

**Integral (`integral(f,a,b)`).** Adaptive Gauss-Kronrod, the range is first split into 8 panels, at most 96 intervals in total (shared by nested integrals). The result is shown only when the error estimate meets the tolerance (1E-12 relative, with a floor tied to the integrand size). If the 80-interval limit is reached without meeting it, the answer is **Math ERROR**; no number is shown. Consequences: divergent integrals (1/X over a range containing 0, X^-1.1 from 0, a pole inside the range) give Math ERROR; so do some convergent but hard integrals: strongly oscillatory (cos(X) on 0..1000), very wide (1/X on 1..1E50) and strongly singular at an endpoint (X^-0.9 on 0..1). Measured on a 65-integral corpus against mpmath: every returned value agrees to better than 1E-8; every divergent case is an error. *Narrow peaks (D15):* if every sample is exactly zero the answer is Math ERROR (this also refuses an identically-zero integrand). The integral is run twice (8 and 11 panels) and Math ERROR is shown if the runs disagree by more than 1E-9 relative. A 731-case Gaussian-peak sweep gave no wrong number (494 correct, 237 Math ERROR). A peak hidden between samples while the samples are non-zero cannot be excluded by any sampling method (documented).

**Derivative (`ddx(f,x)`).** Ridders extrapolation. For |x| >= 1 the first step is 0.05 |x|; for 0 < |x| < 1 two passes (steps 0.05 and 0.05 |x|) are made and the one with the smaller relative error estimate is used, so tiny arguments keep full accuracy (X^2 at 1E-40 gives 2E-40). f must be defined at x itself (1/X at 0 is Math ERROR) and the error estimate must be within 1E-3 of the result, otherwise Math ERROR (a pole or jump within reach of the step: 1/(X-1) at 1.001, Int(X) at 1).

**Solve (`solve(f=g,guess)`).** Sign-change search outward from the guess (step 0.01 relative, growing 1.3x per step, up to about 1E50), Brent refinement, then Newton with multiplicity 1-4 for even-order roots; the steps are relative to the iterate and to the guess scale. **Every candidate root is verified:** |f(r)| <= 1E-4 times the larger of |f| at r(1 +- 1E-3) (at 0: 1E-3 of the guess scale), and f must be non-zero on every side where it is defined. A flat or underflowed function (e^X = 0, X = X+1E-20) therefore gives **Can't Solve**, as do poles and jumps and equations whose solution is a whole interval (Int(X)=2). A root below 1E-30 of the guess scale that f can no longer resolve (X^2 = 0) is reported as 0.

**Quadratic.** When b^2 much larger than 4ac the small root is taken from x1 x2 = c/a (no cancellation). A conjugate pair always lists the root with the positive imaginary part first.

**Sums and products.** Up to 100001 terms (Time Out beyond); bounds must be integers.

### 3.4 Remaining numerical limits

`tests/known_defects/calculus.txt` (not run by ctest): now empty; the former narrow-peak defects are regression cases. The Solve search may take up to about 6,000 function evaluations when there is no root: about 70 ms on the host; the target time is unmeasured (decision D17).

### 3.5 Persistence and frozen identifiers (decision D8)

Saved replay history stores tokens as frozen wire IDs (`core/parser/token_ids.h`; ID list `tests/exam/frozen_token_ids.txt`; groups leave reserved gaps; IDs are never reused). Constants, conversions, statistics-variable and variable tables are append-only (`tests/exam/frozen_aux.txt`). `tests/unit/test_token_ids.c` enforces both and checks that a history holding every token survives save and load. Layout changes to the state structure still bump the blob version and discard saved state; they are batched into as few firmware releases as possible. Current blob version: 7.

### 3.6 Evidence rule for UI and settings features (decision D7)

Rows for entry, display, settings and state features (Ans, replay, cursor editing, natural display, stacked fractions, S<=>D, ENG, variables, M+/M-, display and fraction settings, reset, persistence, contrast, auto-off, error messages) need normal, key-path, boundary and invalid evidence, but no numerical oracle: the independent check is a direct assertion of displayed text, editor content or state (`tests/ui/state.txt`, `state:` form, tagged `@evidence`). All other rows keep the oracle requirement.

## 4. Resource budget (re-measured 2026-11-06 with the capacity limits in place; `tools/check_budget.sh`, run in CI)

| Resource | Now | Ceiling | Headroom |
|---|---|---|---|
| Flash (core only, -Os Cortex-M0+) | 129,659 B | 192 KB (196,608) | 34% |
| RAM: `app_t` (state, history, editor) | 16,368 B | 20 KB | - |
| RAM: `prog_t` compile buffer (inside the static buffers) | 4,240 B | - | - |
| RAM: core static buffers | 15,152 B | - | - |
| RAM: worst-case stack (host-painted, 24-level nesting, nested integral) | 11,512 B | 12,288 B | **6%** |
| LCD buffer (192x64, 1 bit) | 1,536 B | - | - |
| **Total RAM** | **45,596 B (44.5 KB)** | 48 KB (49,152) | 7% (3,556 B) |

Cost of the capacity targets: about +2.4 KB RAM compared with the 11-bracket / 40-literal build (40.4 KB -> 42.7 KB): literal pool 80 x compact `lit_t`, 256-token editor, 384-instruction code buffer. The parser's former 1.5 KB frame was removed by in-place rotation, which kept the worst stack from growing. Conclusions: the targets fit the 48 KB ceiling, so an MCU with at least 64 KB RAM and 256 KB flash remains the minimum. **Stack headroom (6%, after D15) is below the 25% goal in hardware plan HW-3**; all stack figures are host-painted estimates and must be repeated on the device (risk R4/R5).

## 5. Release gates (all required)

1. Every CORE capability TESTED in the register: normal, boundary, invalid input, accuracy (independent oracle), key-sequence, regression.
2. Boundary tests committed for every documented limit.
3. Guard tests for product boundaries (section 1.6) - implemented.
4. Pilot pack all pass; independent second review of expected answers on a sample.
5. Target-hardware run of the full suite, with measured timing and stack high-water mark.
6. Casio reference items verified against the official user guides (**done 2026-11-04**, `02-casio-gap-analysis.md`).
7. WAEC enquiry made and answered in writing before any "approved" or "compliant" statement.
