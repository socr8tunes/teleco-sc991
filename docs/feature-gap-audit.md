# Feature gap audit - SC991 vs fx-991-class calculators

Sources: Casio fx-570/991ES PLUS manual (calculation modes) and the fx-991EX product page (552 functions as stated by Casio).
"Present" means exercised through the key interface in this repository's tests. "Missing" was checked in the lexer, menus and keymap.
This compares published feature lists only; no Casio firmware or manuals were copied, and the SC991 uses its own layout, names and branding.

## Present
Arithmetic, brackets, fractions (mixed/improper), DMS, reciprocal, %, factorial, pi, e, surds, Rnd/Int/Intg/Abs, trig/inverse/hyperbolic in D/R/G, ln/log/log base b/exp/10^x, cube root, n-th root,
nPr/nCr, Pol/Rec, Ran#/RanInt, summation, d/dx, integral, Solve, complex numbers (Arg, Conjg, Re, Im, polar/rectangular), matrices (Det, Trn, Iden, inverse, products), vectors (dot, cross, angle, unit, abs),
statistics (1-var, regression types, estimates), normal distribution P/Q/R, quadratic/cubic/2- and 3-unknown solvers, base-n and logic, constants, unit conversions, Ans/PreAns, 9 variables, M+/M-, replay history, Fix/Sci/Norm/Eng, S<=>D, 2-key rollover.

## Missing or partial (priority for a commercial product)
| # | Feature (fx-991 class) | State | Priority |
|---|---|---|---|
| 1 | TABLE mode: f(x) value table over a range | missing | high |
| 2 | Distributions: binomial and Poisson (pdf/cdf) and inverse normal | missing (normal P/Q/R only) | high |
| 3 | Polynomial equations degree 4; simultaneous equations in 4 unknowns | missing (degree <= 3, <= 3 unknowns) | medium |
| 4 | Inequality solver (degree 2-4) | missing | medium |
| 5 | Prime factorisation; GCD / LCM; integer quotient and remainder | missing | high (school use) |
| 6 | Product (Pi) operator | missing | medium |
| 7 | Ratio solver (A:B = X:D) | missing | low |
| 8 | CALC (re-evaluate an expression with new variable values) | missing | medium |
| 9 | UNDO and multi-statement ":" entry | ":" token exists in the lexer but is not reachable as a working multi-statement feature | medium |
| 10 | Spreadsheet mode (5 x 45) | missing | low-medium |
| 11 | Engineering symbols (k, M, m, n...) in input; digit separators | missing | low |
| 12 | Complex asin/acos/atan; 10^x of a complex number | missing | low |
| 13 | QR code / online graphing | missing | out of scope (needs connectivity) |
| 14 | Full 3rd-order x4 vector memory and 4x4 matrices | matrix size limit to be confirmed against MAT_MAX; 9 vector slots exist | check |

## Recommendation
Close items 1, 2, 5 first (most used in school and exams), then 8, 9, 3, 4, 6. Each needs: spec, implementation, UI keys, tests through the key interface,
and entries in the pilot pack with independently verified answers. Do not describe the product as covering "all functions" until the audit table is closed.
