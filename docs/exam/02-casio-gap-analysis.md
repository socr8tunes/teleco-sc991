# 02 - Casio-reference gap analysis (SC991 Exam vs fx-991ES PLUS / fx-991EX)

Status date: 2026-11-06 (reference verification completed 2026-11-04). Repository state: `main` after the P0 corrections (42 test suites pass). The quartic engine is parked on `wip/quartic-parked` and is **not** part of this analysis' "present" column.

## Reference sources and confidence

| Tag | Meaning | Sources |
|---|---|---|
| **Y** | Confirmed on a Casio page fetched during this audit | [fx-991ES PLUS product page](https://edu.casio.com/products/ntd/fx991esplus/) (417 functions; equation, matrix, vector, complex, CALC, SOLVE, table, integral, derivative, base-n, stat, 9 variables, 40 constants, 40 conversions); [Casio ClassWiz comparison](https://edu.casio.com/products/cwiz/comparison.html) (fx-991EX: 552 functions, table, spreadsheet, QR, inequality, ratio, distributions, up to 4 unknowns and degree 4); [fx-991EX Quick Start Guide](https://edu.casio.com/support/qsg/pdf/991EX_570EX/All_CASIO_QuickStartGuide_fx-991EX_fx-570EX.pdf) (Distribution = normal, inverse normal, binomial, Poisson; Table f(x)/g(x); matrix to 4x4; vectors 2-3 dim; spreadsheet 5x45; engineering symbols; SOLVE by Newton) |
| **N** | Not present in the official guide for that model | Same guides (log below) |
| **?** | Not verified (low impact; no comparative claim made) | - |

### Verification log (P0 item: Casio reference verification)

Y/N flags were checked against the official user guides, not product marketing pages:

- fx-991ES PLUS: [User's Guide, Casio support site](https://support.casio.com/global/en/calc/manual/fx-115ESPLUS_991ESPLUSC_en/before_using_calculator/about.html) (2nd edition, fx-115ES PLUS / fx-991ES PLUS C)
- fx-991EX: [fx-991EX User Guide, Casio Education UK](https://education.casio.co.uk/resources/manuals/fx-991EX-User-Guide/) and the [Quick Start Guide](https://edu.casio.com/support/qsg/pdf/991EX_570EX/All_CASIO_QuickStartGuide_fx-991EX_fx-570EX.pdf)

| Item previously "?" | Result |
|---|---|
| GCD, LCM, Int÷ (Quot), Rmdr (Rem) | **Not in either guide.** Present in the SC991 as extras (Optional). Do not list as Casio parity. |
| Prime factorisation | EX: yes, positive integers up to 10 digits (SC991 limit 9 999 999 999 is the same). No section in the ES PLUS guide. |
| Matrix | ES PLUS: 3 matrices (MatA-C), up to 3x3. EX: 4 matrices + MatAns, up to 4x4. SC991: 4 + Ans, 4x4 (matches EX). |
| Vector | ES PLUS: 3 vectors; EX: 4; both 2-3 dimensions. |
| Statistics rows | ES PLUS: 80 rows (1-variable), 40 (paired), 26 (with frequency). EX: 160 / 80 / 53. SC991: 80 rows in every mode, so it matches ES PLUS for 1-variable and exceeds it for paired data; it is below EX for 1-variable. |
| TABLE | ES PLUS: f(x), 30 rows. EX: f(x) 45 rows, or f(x),g(x) 30 rows. Approved Exam rule (one f(X), at most 30 rows) matches the ES PLUS. |
| CALC | In both guides. |
| Multi-statement ":" | In both guides. Removed from the SC991 by decision; see decision D3 in 07. |
| Engineering symbols, digit separator | EX only. |
| UNDO | EX only (cancels the last key operation, natural-display modes). Not in the ES PLUS guide. |
| Normal PD and general CD | EX only. |

Casio's own function counts: 417 (ES PLUS) and 552 (EX) are Casio marketing figures with an unpublished counting rule. They are not used anywhere in SC991 material.

No WAEC technical specification for calculators was found in public sources searched. The exam-approval criteria must be obtained from WAEC directly (see 05-risks-and-priorities.md).
Casio's guides were used only to list capabilities; no Casio text, layout or branding is reproduced.

## Mode-by-mode comparison

| Casio capability | ES PLUS | EX | SC991 on main | Gap | Exam decision |
|---|:-:|:-:|---|---|---|
| Arithmetic, fractions, mixed numbers, surds, pi multiples, S<=>D | Y | Y | Present, tested | none | Core |
| Scientific functions (trig/inverse/hyperbolic in D/R/G, log, ln, e^x, 10^x, roots, powers, factorial, nPr, nCr, Pol/Rec, Ran#, RanInt, Abs, Int, Intg, Rnd) | Y | Y | Present; boundary and error oracle cases added in P0 (`oracle_functions.txt`) | residual depth, see 04 | Core | Core |
| 9 variables, Ans, PreAns, M+/M-, replay | Y | Y | Present | replay depth (480 tokens) and expression length (below) | Core |
| Constants (40) and conversions (40 entries) | Y | Y | 40 CODATA-2022 constants, 40 directional conversions | none vs ES PLUS | Core |
| Summation | Y | Y | Present | - | Core |
| Product | N | Y | Present (Pi) | - | Optional |
| Derivative, definite integral | Y | Y | Present | - | Core |
| SOLVE | Y | Y | Present (Newton + bracket scan) | - | Core |
| **CALC** | Y | Y | **Missing** (removed 2026-11-01; restoration approved as session-only single expression; **on hold until P0 review**) | rebuild | Core |
| **TABLE** | f(x) | f(x), g(x) | **Missing** (restoration approved; one f(X); **on hold until P0 review**) | rebuild | Core |
| Equation: simultaneous | 2-3 | 2-4 | 2, 3, 4 unknowns present; coefficient grid with correction | needs independent tests on main for grid UI edge cases | Core |
| Equation: polynomial | deg 2-3 | deg 2-4 | deg 2-3 on main (quadratic small-root cancellation fixed in P0); **quartic engine parked**, unverified | quartic | Core |
| **Inequality** | N | deg 2-4 | **Missing** (approved, on hold) | build | Core (EX parity; useful for SHS/university algebra) |
| Ratio A:B=X:D | N | Y | Missing | build (small) | Optional |
| Complex (arithmetic, Arg, Conjg, Re, Im, polar/rect) | Y | Y | Present; asin/acos/atan and 10^z of complex unsupported | minor | Core / Optional |
| Matrix | 3 mats, 3x3 | 4 mats, 4x4 | 4 named + Ans, up to 4x4, Det/Trn/Iden/inverse/product | test depth; EX extras (Ref/Rref etc.) not in scope | Core |
| Vector | 3 vecs, 2-3 dim | 4 vecs, 2-3 dim | 4 named + Ans, 2-3 dim; Dot, Cross, Angle, UnitV, Abs | test depth | Core |
| Statistics: 1-var, paired, 7 regressions, frequency | Y | Y | Present, 80 rows | none functional | Core |
| Normal distribution P/Q/R (standard) | Y | Y (as Normal PD/CD) | Present; **no normal density PD, no general CD(lower, upper, sigma, mu)** | add | Optional (recommended) |
| Inverse normal | N | Y | Present (InvN) | - | Core for university |
| Binomial / Poisson PD, CD | N | Y | Present | EX list-table output not provided | Optional |
| Base-n and logic (and/or/xor/xnor/Not/Neg) | Y | Y | Present; hand-verified only | oracle + key-path tests | Core |
| Prime factorisation | Y | Y | Present | - | Core |
| GCD, LCM, Quot, Rem | N | N | Present (SC991 extra) | none; do not claim as Casio parity | Optional |
| Engineering symbols in input, digit separator | N | Y | Missing | build (low cost) | Optional |
| UNDO | N | Y | Missing | decide (D6 in 07) | Optional |
| Multi-statement (":") | Y | Y | Removed by product decision | revisit only with WAEC input | Out |
| Spreadsheet | N | Y | Excluded | - | Out |
| QR code | N | Y | Excluded | - | Out (needs connectivity) |

## Counts

Capabilities catalogued: see `01-feature-inventory.md` and `04-coverage-register.md` (generated, reproducible). The SC991 offers **295** distinct reachable entries under the counting rule stated there. Casio's 417 / 552 use an unpublished rule and **cannot be reconciled** with any count of ours. The repository never defined a "417 target"; the number appears to have been taken from Casio's ES PLUS marketing. It has been removed from all customer-facing material (README, pilot pack, docs, web page: none contained it on re-check) and survives only here and in the risk register as a rejected claim. No function count is to be quoted until a signed-off function list exists.

## Where the SC991 is ahead of or different from the references

- Natively exact arithmetic to 45-54 digits internally (xnum) with 16-digit displayed arithmetic; Casio publishes 15-digit internal precision for the ES PLUS (not verified).
- Linear systems with exact rational Bareiss elimination and explicit unique / inconsistent / underdetermined results.
- Statistics editor holds 80 paired rows (Casio ES PLUS limits paired data to 40; EX to 80 - verified).

## Where it is weaker, independent of features

- Entry capacity (fixed in P0): now 24 nested levels, 80 literals, 256 tokens per line (`limits-probe.md`, spec section 3). Casio EX accepts 159 bytes of entry.
- Error and boundary tests: large oracle suites added in P0; residual gaps are listed in `04-coverage-register.md` and `06-test-results.md`.
- No hardware verification, no measured speed on the target MCU, no exam body review.
