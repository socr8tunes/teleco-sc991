# Feature gap audit - SC991 vs fx-991-class calculators

Sources: Casio fx-570/991ES PLUS manual (calculation modes) and the fx-991EX product page (552 functions as stated by Casio).
"Present" means exercised through the key interface in this repository's tests. "Missing" was checked in the lexer, menus and keymap.
This compares published feature lists only; no Casio firmware or manuals were copied, and the SC991 uses its own layout, names and branding.

## Present
Arithmetic, brackets, fractions (mixed/improper), DMS, reciprocal, %, factorial, pi, e, surds, Rnd/Int/Intg/Abs, trig/inverse/hyperbolic in D/R/G, ln/log/log base b/exp/10^x, cube root, n-th root,
nPr/nCr, Pol/Rec, Ran#/RanInt, summation, d/dx, integral, Solve, complex numbers (Arg, Conjg, Re, Im, polar/rectangular), matrices (Det, Trn, Iden, inverse, products), vectors (dot, cross, angle, unit, abs),
statistics (1-var, regression types, estimates), normal distribution P/Q/R, quadratic/cubic/2- and 3-unknown solvers, base-n and logic, constants, unit conversions, Ans/PreAns, 9 variables, M+/M-, replay history, Fix/Sci/Norm/Eng, S<=>D, 2-key rollover.

## Permanently excluded (SC991 Exam)
- **Spreadsheet mode.**
- **User-programmable formulas, macros and scripting** - nothing that stores user-written logic for reuse.
- Multi-statement ":" chaining stays removed for now (a ":" entry is a Syntax ERROR).
Inline expressions typed into Solve(, d/dx, integral, sum and prod are evaluated once and not stored, so they stay.
TABLE (one f(X), at most 30 rows, session only) and CALC (one expression, session only, never persisted) are **approved for the Exam specification but not implemented**; implementation is on hold until the P0 review closes.
`tests/unit/test_exclusions.c` (ctest `exclusions`) enforces the boundary: the token set must equal `tests/exam/approved_tokens.txt`, every menu item is visited and may insert only approved tokens, and no menu label may name a forbidden capability.

## Missing or partial (priority for a commercial product)
| # | Feature (fx-991 class) | State | Priority |
|---|---|---|---|
| 1 | TABLE mode: f(x) value table over a range | approved for the Exam spec, implementation on hold | after P0 review |
| 2 | Distributions: binomial and Poisson (pdf/cdf) and inverse normal | **done** (MENU > Distribution: InvN, BinomPD/CD, PoisPD/CD; n, lambda <= 10000) | done |
| 3 | Polynomial equations degree 4; simultaneous equations in 4 unknowns | Simul4 **done**; quartic parked (branch `wip/quartic-parked`, untested, not on main) | quartic on hold |
| 4 | Inequality solver (degree 2-4) | approved, on hold | after P0 review |
| 5 | Prime factorisation; GCD / LCM; integer quotient and remainder | **done** (MENU > Number: GCD, LCM, Quot, Rem, PFact; PFact up to 9 999 999 999) | done |
| 6 | Product (Pi) operator | **done** (`prod(`; same range limit as sum) | done |
| 7 | Ratio solver (A:B = X:D) | missing | low |
| 8 | CALC (re-evaluate an expression with new variable values) | approved for the Exam spec, implementation on hold | after P0 review |
| 9 | UNDO and multi-statement ":" entry | ":" chaining stays removed for now | n/a |
| 10 | Spreadsheet mode (5 x 45) | permanently excluded | n/a |
| 11 | Engineering symbols (k, M, m, n...) in input; digit separators | missing | low |
| 12 | Complex asin/acos/atan; 10^x of a complex number | missing | low |
| 13 | QR code / online graphing | missing | out of scope (needs connectivity) |
| 14 | Full 3rd-order x4 vector memory and 4x4 matrices | matrix size limit to be confirmed against MAT_MAX; 9 vector slots exist | check |

## Recommendation
Items 2, 5 and 6 are closed and Simul4 is done. No new feature work until the P0 review (see `docs/exam/`). Do not describe the product as covering "all functions" or quote a function count until the audit table is closed.
