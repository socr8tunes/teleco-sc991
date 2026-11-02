# 03 - Proposed final SC991 Exam feature specification (DRAFT for approval)

Product: SC991 Exam - non-programmable, non-graphing, silent, cordless, self-contained scientific calculator for Ghanaian SHS and university students and general users. Benchmark: Casio fx-991ES PLUS / fx-991EX capabilities (not their branding or interface). Subject to WAEC review; **no approval is claimed**.

## 1. Product boundaries (hard rules, enforced by tests)

1. No user programs, macros, scripts, user-defined functions or persistent executable formulas.
2. No graphing or plotting. TABLE shows numbers only.
3. No spreadsheet, no external data import/export, no QR, no radio/IO of any kind in firmware. (Firmware programming port is a manufacturing feature, disabled or locked in shipped units - hardware decision, see risks.)
4. No symbolic algebra: results are numeric, exact rational, surd, pi-multiple or complex-rectangular/polar only. No expression simplification, no factoring of user expressions, no differentiation or integration to a symbolic form.
5. No AI. No multi-statement chaining. Session-only helpers (CALC, TABLE) keep exactly one expression and are cleared by AC, leaving the mode and power-off; never written to non-volatile storage.
6. Guard test (to be written, see register): static scan of token enum and menu for storage/branch/loop operators, plus a behavioural test that the persisted blob contains no expression other than replay history entries, and that CALC/TABLE content is absent after a save/load cycle.

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

## 3. Numerical and capacity requirements (proposed targets)

| Item | Measured today | Proposed Exam target |
|---|---|---|
| Internal arithmetic | 16-digit decimal, exponent +-99, exact rational/surd tracking, 45-54-digit extended path | keep |
| Display | 10 significant digits (Norm), Fix/Sci/Eng | keep |
| Matrix / vector / stat rows | 4x4 / 3-dim / 80 | keep (boundary tests committed) |
| Bracket nesting | **11** (docs say 24) | at least 24 usable levels |
| Number literals per expression | **40** | at least 80; target equivalent to Casio EX's 159-key entry |
| Tokens per line / compiled | 128 / 160 | at least 160 / 240 (RAM budget permitting) |
| History | 480 tokens | keep; document |
| Accuracy | functions correctly rounded vs mpmath on difftests; expression differential | keep; extend to all CORE functions |
| Run-time | not measured on target | each CORE operation returns within 2 s on the chosen MCU; sums, products, integrals, Solve bounded by Time Out |

## 4. Resource budget (measured on host/cross-compile, 2026-11-02)

| Resource | Now | Proposed ceiling |
|---|---|---|
| Flash (core only, -Os Cortex-M0+) | 124,556 B | 192 KB (so a 256 KB part has margin for HAL, boot, font, options) |
| RAM: app state | 16,112 B | 20 KB |
| RAM: core static buffers | 12,304 B | 12 KB |
| RAM: worst-case stack (host-measured) | 10,468 B | 12 KB (verify on target) |
| LCD buffer (192x64, 1 bit) | 1,536 B (assumed) | - |
| Total RAM | **about 40.4 KB** | 48 KB; MCU with at least 64 KB RAM |

## 5. Release gates (all required)

1. Every CORE capability TESTED in the register: normal, boundary, invalid input, accuracy (independent oracle), key-sequence, regression.
2. Boundary tests committed for every documented limit.
3. Guard tests for product boundaries (section 1.6).
4. Pilot pack all pass; independent second review of expected answers on a sample.
5. Target-hardware run of the full suite, with measured timing and stack high-water mark.
6. Casio reference items marked "?" verified against the official user guides.
7. WAEC enquiry made and answered in writing before any "approved" statement.
