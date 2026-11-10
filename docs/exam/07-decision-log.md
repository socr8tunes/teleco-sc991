# 07 - Decision log (SC991 Exam)

Updated 2026-11-10 after the owner's D14-D17 decisions (2026-11-09); P1 accepted as complete.

## Decisions

| # | Decision | Outcome | Date | Status |
|---|---|---|---|---|
| D1 | Written enquiry to WAEC with the boundary list | Not addressed in the 2026-11-07 instruction. **Still pending.** No approval or compliance wording is used anywhere; engineering does not depend on it, but feature acceptance does (R1). | - | **Pending** | **Deferred by the owner 2026-11-09** until the product is ready for submission. |
| D2 | Release of the hold on approved features | **Approved sequence: CALC, TABLE, inequality solver, quartic solver**, after P1 numerical correctness and test gates are complete. Each feature needs: specification, implementation, key-path tests, independent expected values, boundary and error tests, regression coverage - all before the next starts. Rules already approved: CALC = single expression, session only, never persisted; TABLE = one f(X), at most 30 rows, session only, ordinary math functions only; inequality = one interval per page with k/N indicator and endpoint inclusion; quartic = exact where practical, multiplicity, complex roots, robust method. | 2026-11-07 | **Approved** |
| D3 | Multi-statement ":" entry | **Stays removed.** | 2026-11-07 | **Approved** |
| D4 | Singular and non-convergent integrals | **Conservative non-convergence handling.** The integral is shown only if its error estimate meets the tolerance; otherwise Math ERROR. Implemented (spec 3.3). | 2026-11-07 | **Implemented** |
| D5 | Scale-relative derivative and Solve | **Approved.** Relative step for the derivative, relative-residual verification and plateau rejection for Solve; bogus root of `e^X=0` removed. Implemented (spec 3.3). | 2026-11-07 | **Implemented** |
| D6 | Optional features (Normal PD / general CD, ratio solver, engineering symbols, digit separator, UNDO, complex inverse trig) | **Deferred.** | 2026-11-07 | **Approved** |
| D7 | Coverage-register rule for UI and settings features | **Approved:** no numerical oracle required; evidence is a UI/state assertion (displayed text, editor content, `state:` name=value in `tests/ui/*.txt`, tagged `@evidence`) for normal, key-path, boundary and invalid input. Applied to 16 rows. | 2026-11-07 | **Implemented** |
| D8 | Persistence stability | **Freeze token IDs and reserved gaps before pilot hardware distribution.** Implemented as frozen wire IDs (`core/parser/token_ids.h`), persistence version 7, frozen auxiliary tables, freeze test. | 2026-11-07 | **Implemented** |
| D9 | Stack headroom 13% | **Provisional**; the hardware measurement sets the real margin. | 2026-11-07 | **Approved (provisional)** |
| D10 | MCU class and bring-up | **At least 256 KB flash and 64 KB RAM; proceed with Cortex-M4 bring-up planning.** Plan in `docs/hardware/cortex-m4-bringup-plan.md`; purchases and the proposed gates still need your sign-off. | 2026-11-07 | **Planning done; purchase pending** |
| D11 | GCD, LCM, Quot, Rem | **Remain SC991 extras; never represented as Casio parity.** | 2026-11-07 | **Approved** |
| D12 | Legal review of the key layout | Not addressed in the 2026-11-07 instruction. **Still pending** (R12). | - | **Pending** | Pending to the hardware design stage (owner, 2026-11-09). |
| D13 | Product separation | **No SC991 Pro or SC991 AI functionality in the Exam model.** The exclusion guard test (approved-token list, menu scan) enforces it. | 2026-11-07 | **Approved** |

## Remaining decisions (new)

| # | Decision | Recommendation |
|---|---|---|
| D14 | Which error to show for a valid integral the scheme cannot verify (oscillatory, very wide, strong singularity): Math ERROR now, or Time Out. Casio's guide uses Time Out for slow integrals. | Keep Math ERROR for the pilot; revisit with hardware timing. | **Approved 2026-11-09:** keep Math ERROR for the pilot. |
| D15 | The narrow-peak case (e^X from -1E5 to 0 returns 0) cannot be detected by any sampling quadrature. Document only, or add a guard that refuses an integral whose samples are all exactly zero. | **Revised by the owner 2026-11-09:** silent wrong results are not acceptable as a documented limitation alone. Implemented: 8-panel seed, a second independent 11-panel run that must agree to 1E-9 (a single run returned a 0.2% wrong value on one partly resolved peak), and Math ERROR when every sample is exactly zero (integral(0,...) is refused; Int(X) over [0,0.5] is not). Both former known defects now return 1 correctly. Nested integrals now use a shared interval pool. Remaining limit documented in the pilot limitations. | **Revised 2026-11-09, implemented 2026-11-10.** |
| D16 | Solve with a whole-interval solution (Int(X)=2) returns Can't Solve. | Accept; rare in the syllabus. | **Approved 2026-11-09.** |
| D17 | Solve worst-case time (no root: about 70 ms on the host, 10-30 times that on a Cortex-M4, perhaps over 2 s). | Measure first; time-based Time Out is in the bring-up plan. | **Approved 2026-11-09:** time-based timeout, value set from real MCU measurements, configurable during bring-up (board proposal section 4). |

## New decisions awaiting you (2026-11-10)

| # | Decision | Document | Recommendation |
|---|---|---|---|
| D18 | Approve the CALC specification (questions Q1 key, Q2 stored values, Q3 number-only prompt, Q4 bound variables, Q5 history, Q6 build switch) | `08-calc-specification.md` | Approve as written; implementation starts only on approval |
| D19 | Approve the NUCLEO-L452RE as the primary bring-up board (about AUD 29), L432KC held back as the final exact-fit check; display and keypad parts to be quoted | `../hardware/board-proposal.md` | Approve; nothing is bought until you say so |

