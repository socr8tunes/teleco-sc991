# 05 - Technical risks and recommended implementation priorities (revised 2026-11-07, after P1)

Severity: H = could block release or mislead customers; M = significant rework or quality risk; L = manageable. Status: **Closed** = fixed and evidenced; **Reduced** = improved, residual risk stated; **Open** = unchanged.

## Risk register

| # | Risk | Sev | Status | Evidence / what changed | Next action |
|---|---|---|---|---|---|
| R1 | **Exam acceptance unknown.** No WAEC technical specification was found; Casio parity does not guarantee acceptance (CALC, TABLE, matrix, complex, base-n, replay, persistent memory could each be questioned). No approval or compliance claim is made anywhere. | H | **Open** | wording corrected; the WAEC enquiry (D1) was **not addressed** in the 2026-11-07 instruction and remains pending | Decide D1; CALC and TABLE are the features most likely to be questioned, so the enquiry should precede their release |
| R2 | **"417 functions" unsupported.** Casio's ES PLUS marketing number; our own rule counts 295 entries. | H | **Closed** | Re-checked README, docs, pilot pack, web page: no customer-facing use. Remains only in internal audit text as a rejected claim | Quote no function count until a signed-off list exists |
| R3 | **Entry capacity weak** (was 11 levels / 40 literals). | H | **Closed** | 24 levels, 80 literals, 256 tokens implemented; boundary tests at every limit; budget re-validated (spec 3.1, 4) | - |
| R4 | **No hardware evidence.** Host speed is not MCU speed; Sigma/Pi allow 100001 terms, integrals 80 intervals, Solve about 6,000 evaluations when there is no root (70 ms on the host); Time Out is a step count, not time. | H | **Open** | bring-up plan written (`docs/hardware/cortex-m4-bringup-plan.md`) with measurements, gates and a time-based Time Out; no hardware used yet | Approve the plan and purchase (D10); measure step rate first |
| R5 | **Memory larger than first documented.** | H | **Reduced** | Docs corrected: 128.3 KB flash, 43.7 KB RAM, stack 10.7 KB (host). Budget now enforced in CI (`tools/check_budget.sh`). Headroom: RAM 11%, flash 36%, stack 13% | Choose MCU with 256 KB flash and at least 64 KB RAM; re-measure on device |
| R6 | **Test depth uneven.** | M | **Reduced** | P1: 135 numerical cases (integrals, derivatives, Solve) plus 27 named regression cases, evidence rule applied to 16 UI/settings rows with 87 UI/state cases, menu key-path and exclusion tests, valgrind in CI. Register: 78 of 116 implemented capabilities meet the rule (31 at the start of P0, 62 after P0); 36 remain (see 06) | Close the remaining rows with boundary / invalid / key-path cases, or after the features are built |
| R7 | **Documentation overstates.** | M | **Closed** | Guard test now exists; bracket/literal/memory figures corrected; audit table and TABLE/CALC/Pi/Simul4 statuses updated | - |
| R8 | **Non-programmability must be demonstrable.** CALC/TABLE hold an expression; replay history persists. | M | **Open** | Guard test enforces the token and menu boundary; CALC/TABLE not yet built | Session-only persistence test when built; explain in the WAEC submission |
| R9 | **Saved-state format churn.** | M | **Reduced** | token IDs frozen (wire IDs, persistence v7, aux tables append-only, freeze test). State-structure layout changes still discard saved state | Batch layout changes; add a migration only if requested |
| R10 | **Casio reference partly unverified.** | M | **Closed** | Verified against official guides 2026-11-04; log in 02. GCD/LCM/Quot/Rem are SC991 extras, not Casio parity | - |
| R11 | **Quartic engine unverified.** | M | **Open (parked)** | on `wip/quartic-parked`, not on main | Stays parked per instruction |
| R12 | **Look-alike layout** (Casio ES layout). | M | **Open** | unchanged | Legal review; consider layout differentiation |
| R13 | **Syllabus fit untested.** | M | **Open** | unchanged | Map WAEC SHS syllabi to capabilities; teacher-reviewed questions |
| R14 | **Firmware extraction/programming port.** | M | **Open** | not designed | Add readout protection / locked bootloader to hardware requirements |
| R15 | **Stack headroom below the 25% goal.** Worst host-painted stack 10,692 B of 12,288 B (13%) after the 24-level target. | M | **Open (provisional, D9)** | the numerical rewrite did not change the worst case (re-measured in CI) | Hardware measurement sets the real margin (plan section 4) |
| R16 | **Numerical-method edge defects.** | M | **Reduced** | integrals never show an unverified number (Math ERROR); derivatives and Solve use relative steps and verified roots; bogus roots removed; 135 oracle cases. Residual: narrow-peak integrals can return 0 (D15); valid but hard integrals give Math ERROR (D14); whole-interval Solve gives Can't Solve (D16) | Decisions D14-D16 |
| R17 | **Defects found and fixed in P0.** Quadratic small-root cancellation (`quad(1,1E10,1)` returned 0 instead of -1E-10); conjugate pair order depended on sign of a; entry of -2147483648 in decimal base-n; vector angle of a zero vector; `r` of constant-y regression (now Math ERROR); uninitialised output on underflow in numeric integration (valgrind-confirmed). | - | **Closed** | pinned by oracle cases | - |
| R18 | **Register rule vs UI features.** | L | **Closed** | evidence rule approved (D7) and applied; 16 rows moved to tested | - |
| R19 | **Conservative integral errors may frustrate users.** Valid oscillatory or wide-range integrals now give Math ERROR instead of a number. | M | **New** | 7 such cases listed in `numerical_integrals.txt`; documented in the pilot limitations | Teacher sample; D14 |
| R20 | **Solve time on the target.** No-root Solve evaluates f up to about 6,000 times. | M | **New** | 70 ms on the host | Measure on hardware; time-based Time Out (D17) |
| R21 | **Numerical changes invalidate earlier pilot expectations.** | L | **Closed** | pilot bank and all suites pass; no pilot answer changed | - |

## Recommended priorities

**P0 - complete** (2026-11-06). Documentation corrections, Casio verification, entry-capacity targets, test-debt closure, budget in CI.

**P1 - complete** (2026-11-07): conservative integrals, relative derivative and Solve with verified roots, regression and oracle tests for every identified numerical defect, evidence rule for UI/settings features, frozen token IDs and persistence version 7, valgrind in CI, Cortex-M4 bring-up plan, decision log.

**P2 - approved sequence, one feature at a time** (D2). Each needs a specification, implementation, key-path tests, independent expected values, boundary and error tests and regression coverage before the next starts:
1. CALC (single expression, session only, never persisted)
2. TABLE (one f(X), at most 30 rows, session only)
3. Inequality solver (one interval per page, k/N, endpoint inclusion)
4. Quartic solver (from the parked branch; exact where practical, multiplicity, complex roots)

**In parallel**: Cortex-M4 bring-up after your sign-off on purchase and gates (D10).

**Deferred (D6)**: Normal PD / general CD, ratio solver, engineering symbols, digit separator, UNDO, complex inverse trig. ":" chaining stays removed (D3).

## Test plan summary

For each CORE capability the register must show: normal, boundary, invalid input, accuracy against an independent oracle, key-sequence, and regression coverage. `tools/audit/register.py --check` runs in CI so new lexer words cannot ship without a catalogue row. The approved-token list makes any new token a deliberate, reviewed change.
