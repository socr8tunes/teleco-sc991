# 02 - Casio-reference gap analysis (SC991 Exam vs fx-991ES PLUS / fx-991EX)

Status date: 2026-11-02. Repository state: `main` at 53ac43c (26 test suites pass). The quartic engine is parked on `wip/quartic-parked` and is **not** part of this analysis' "present" column.

## Reference sources and confidence

| Tag | Meaning | Sources |
|---|---|---|
| **Y** | Confirmed on a Casio page fetched during this audit | [fx-991ES PLUS product page](https://edu.casio.com/products/ntd/fx991esplus/) (417 functions; equation, matrix, vector, complex, CALC, SOLVE, table, integral, derivative, base-n, stat, 9 variables, 40 constants, 40 conversions); [Casio ClassWiz comparison](https://edu.casio.com/products/cwiz/comparison.html) (fx-991EX: 552 functions, table, spreadsheet, QR, inequality, ratio, distributions, up to 4 unknowns and degree 4); [fx-991EX Quick Start Guide](https://edu.casio.com/support/qsg/pdf/991EX_570EX/All_CASIO_QuickStartGuide_fx-991EX_fx-570EX.pdf) (Distribution = normal, inverse normal, binomial, Poisson; Table f(x)/g(x); matrix to 4x4; vectors 2-3 dim; spreadsheet 5x45; engineering symbols; SOLVE by Newton) |
| **?** | From general knowledge, **not yet verified** | Must be checked against the full fx-991ES PLUS and fx-991EX user guides before any comparative claim. Examples: GCD/LCM/Int÷/Rmdr availability on each model, ES PLUS matrix/vector/statistics row limits, EX constant count |

No WAEC technical specification for calculators was found in public sources searched. The exam-approval criteria must be obtained from WAEC directly (see 05-risks-and-priorities.md).
Casio's guides were used only to list capabilities; no Casio text, layout or branding is reproduced.

## Mode-by-mode comparison

| Casio capability | ES PLUS | EX | SC991 on main | Gap | Exam decision |
|---|:-:|:-:|---|---|---|
| Arithmetic, fractions, mixed numbers, surds, pi multiples, S<=>D | Y | Y | Present, tested | none | Core |
| Scientific functions (trig/inverse/hyperbolic in D/R/G, log, ln, e^x, 10^x, roots, powers, factorial, nPr, nCr, Pol/Rec, Ran#, RanInt, Abs, Int, Intg, Rnd) | Y | Y | Present; error-case and boundary tests thin | test depth only | Core |
| 9 variables, Ans, PreAns, M+/M-, replay | Y | Y | Present | replay depth (480 tokens) and expression length (below) | Core |
| Constants (40) and conversions (40 entries) | Y | Y (EX constant count ?) | 40 CODATA-2022 constants, 40 directional conversions | none vs ES PLUS | Core |
| Summation | Y | Y | Present | - | Core |
| Product | N | Y | Present (Pi) | - | Optional |
| Derivative, definite integral | Y | Y | Present | - | Core |
| SOLVE | Y | Y | Present (Newton + bracket scan) | - | Core |
| **CALC** | Y | ? | **Missing** (removed 2026-11-01; restoration approved as session-only single expression) | rebuild | Core |
| **TABLE** | f(x) | f(x), g(x) | **Missing** (restoration approved; one f(X)) | rebuild | Core |
| Equation: simultaneous | 2-3 | 2-4 | 2, 3, 4 unknowns present; coefficient grid with correction | needs independent tests on main for grid UI edge cases | Core |
| Equation: polynomial | deg 2-3 | deg 2-4 | deg 2-3 on main; **quartic engine parked**, unverified | quartic | Core |
| **Inequality** | N | deg 2-4 | **Missing** | build | Core (EX parity; useful for SHS/university algebra) |
| Ratio A:B=X:D | N | Y | Missing | build (small) | Optional |
| Complex (arithmetic, Arg, Conjg, Re, Im, polar/rect) | Y | Y | Present; asin/acos/atan and 10^z of complex unsupported | minor | Core / Optional |
| Matrix | 3 mats, 3x3 (?) | 4x4 | 4 named + Ans, up to 4x4, Det/Trn/Iden/inverse/product | test depth; check EX extras (e.g. Ref/Rref, abs/norm) against the user guide | Core |
| Vector | 3 vecs (?) | 2-3 dim | 4 named + Ans, 2-3 dim; Dot, Cross, Angle, UnitV, Abs | test depth | Core |
| Statistics: 1-var, paired, 7 regressions, frequency | Y | Y | Present, 80 rows | none functional | Core |
| Normal distribution P/Q/R (standard) | Y | Y (as Normal PD/CD) | Present; **no normal density PD, no general CD(lower, upper, sigma, mu)** | add | Optional (recommended) |
| Inverse normal | N | Y | Present (InvN) | - | Core for university |
| Binomial / Poisson PD, CD | N | Y | Present | EX list-table output not provided | Optional |
| Base-n and logic (and/or/xor/xnor/Not/Neg) | Y | Y | Present; hand-verified only | oracle + key-path tests | Core |
| Prime factorisation | Y | Y | Present | - | Core |
| GCD, LCM, Quot, Rem | ? | ? | Present | verify Casio availability only for the comparison table | Optional |
| Engineering symbols in input, digit separator | ? | Y | Missing | build (low cost) | Optional |
| UNDO | ? | ? | Missing | decide | Optional |
| Multi-statement (":") | Y (ES) | ? | Removed by product decision | revisit only with WAEC input | Out |
| Spreadsheet | N | Y | Excluded | - | Out |
| QR code | N | Y | Excluded | - | Out (needs connectivity) |

## Counts

Capabilities catalogued: see `01-feature-inventory.md` and `04-coverage-register.md` (generated, reproducible). The SC991 offers **295** distinct reachable entries under the counting rule stated there. Casio's 417 / 552 use an unpublished rule and **cannot be reconciled** with any count of ours. The repository never defined a "417 target"; the number appears to have been taken from Casio's ES PLUS marketing. Recommendation: stop using 417 in any customer-facing text until a signed-off function list exists.

## Where the SC991 is ahead of or different from the references

- Natively exact arithmetic to 45-54 digits internally (xnum) with 16-digit displayed arithmetic; Casio publishes 15-digit internal precision for the ES PLUS (**?**, from memory).
- Linear systems with exact rational Bareiss elimination and explicit unique / inconsistent / underdetermined results.
- Statistics editor holds 80 paired rows (Casio ES PLUS limits paired data to 40, **?**).

## Where it is weaker, independent of features

- Entry capacity: 11 nested bracket levels and 40 number literals per expression (measured, `limits-probe.md`). A student typing a 41-number sum gets Stack ERROR.
- Error and boundary tests are thin for calculus, statistics, matrices, vectors, base-n and complex numbers (register).
- No hardware verification, no measured speed on the target MCU, no exam body review.
