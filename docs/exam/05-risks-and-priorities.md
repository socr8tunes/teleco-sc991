# 05 - Technical risks and recommended implementation priorities (revised 2026-11-06, after P0)

Severity: H = could block release or mislead customers; M = significant rework or quality risk; L = manageable. Status: **Closed** = fixed and evidenced; **Reduced** = improved, residual risk stated; **Open** = unchanged.

## Risk register

| # | Risk | Sev | Status | Evidence / what changed | Next action |
|---|---|---|---|---|---|
| R1 | **Exam acceptance unknown.** No WAEC technical specification was found; Casio parity does not guarantee acceptance (CALC, TABLE, matrix, complex, base-n, replay, persistent memory could each be questioned). No approval or compliance claim is made anywhere. | H | **Open** | README, pilot docs and hardware plan now say "no approval or compliance is claimed" | Written enquiry to WAEC with the boundary list (spec sections 1-2); decision D1 |
| R2 | **"417 functions" unsupported.** Casio's ES PLUS marketing number; our own rule counts 295 entries. | H | **Closed** | Re-checked README, docs, pilot pack, web page: no customer-facing use. Remains only in internal audit text as a rejected claim | Quote no function count until a signed-off list exists |
| R3 | **Entry capacity weak** (was 11 levels / 40 literals). | H | **Closed** | 24 levels, 80 literals, 256 tokens implemented; boundary tests at every limit; budget re-validated (spec 3.1, 4) | - |
| R4 | **No hardware evidence.** Host speed is not MCU speed; integrals/sums/Solve allow up to 100001 terms / 4,000,000 VM steps; Time Out is a count, not time. | H | **Open** | unchanged | Cortex-M4 bring-up; measure latency and stack; set Time Out by time |
| R5 | **Memory larger than first documented.** | H | **Reduced** | Docs corrected: 125.2 KB flash, 43.7 KB RAM, stack 10.7 KB (host). Budget now enforced in CI (`tools/check_budget.sh`). Headroom: RAM 11%, flash 36%, stack 13% | Choose MCU with 256 KB flash and at least 64 KB RAM; re-measure on device |
| R6 | **Test depth uneven.** | M | **Reduced** | ~2,800 independent-oracle cases added (matrix, vector, complex, base-n x4, domain, stats errors, calculus, equations, scientific edges), menu key-path test, exclusion guard, capacity tests. Register: 62 of 116 implemented capabilities now meet the strict rule (was 31); 52 remain (see 06) | Close remaining gaps after review, or relax the oracle requirement for UI features (D7) |
| R7 | **Documentation overstates.** | M | **Closed** | Guard test now exists; bracket/literal/memory figures corrected; audit table and TABLE/CALC/Pi/Simul4 statuses updated | - |
| R8 | **Non-programmability must be demonstrable.** CALC/TABLE hold an expression; replay history persists. | M | **Open** | Guard test enforces the token and menu boundary; CALC/TABLE not yet built | Session-only persistence test when built; explain in the WAEC submission |
| R9 | **Saved-state format churn** (token codes shift; persistence version bump wipes data; VERSION 6). | M | **Open** | unchanged | Freeze token enum / stable IDs before first customer release (D8) |
| R10 | **Casio reference partly unverified.** | M | **Closed** | Verified against official guides 2026-11-04; log in 02. GCD/LCM/Quot/Rem are SC991 extras, not Casio parity | - |
| R11 | **Quartic engine unverified.** | M | **Open (parked)** | on `wip/quartic-parked`, not on main | Stays parked per instruction |
| R12 | **Look-alike layout** (Casio ES layout). | M | **Open** | unchanged | Legal review; consider layout differentiation |
| R13 | **Syllabus fit untested.** | M | **Open** | unchanged | Map WAEC SHS syllabi to capabilities; teacher-reviewed questions |
| R14 | **Firmware extraction/programming port.** | M | **Open** | not designed | Add readout protection / locked bootloader to hardware requirements |
| R15 | **Stack headroom below the 25% goal.** Worst host-painted stack 10,692 B of 12,288 B (13%) after the 24-level target. | M | **New** | `bench_stack` (ctest `stack_budget`) | Re-measure on target; options: raise ceiling, shrink `val_t`/solver frames |
| R16 | **Numerical-method edge defects.** Calculus loses 1-2 digits on oscillatory/singular integrals; derivative step and Solve tolerance are absolute (tiny arguments); `solve(e^X=0)` can return a bogus root; singular integrals have no clean non-convergence test. | M | **New (documented)** | `tests/known_defects/calculus.txt`, spec 3.3 | Decisions D4, D5 |
| R17 | **Defects found and fixed in P0.** Quadratic small-root cancellation (`quad(1,1E10,1)` returned 0 instead of -1E-10); conjugate pair order depended on sign of a; entry of -2147483648 in decimal base-n; vector angle of a zero vector; `r` of constant-y regression (now Math ERROR); uninitialised output on underflow in numeric integration (valgrind-confirmed). | - | **Closed** | pinned by oracle cases | - |
| R18 | **Register rule vs UI features.** The strict rule demands an independent oracle even for display/settings features where an oracle has no meaning. | L | **Open** | about 15 of the 52 remaining rows | Decision D7 |

## Recommended priorities

**P0 - complete (this package).** Documentation corrections; Casio verification; capacity targets implemented; test-debt closure for matrices, vectors, complex, base-n, domain, statistics errors, calculus, equations and scientific functions; menu key-path test; exclusion guard; budget validation in CI; full local CI (gcc, clang, sanitizers, wasm, footprint, budget, template) green; valgrind clean on the new case files.

**P1 - after your review and approvals (nothing started)**
1. CALC and TABLE per the approved rules (session-only).
2. Inequality solver (one interval per page, k/N indicator, endpoint inclusion).
3. Quartic from the parked branch, only with the approved verification plan.
4. Hardware bring-up and timing (can run in parallel; resolves R4, R5, R15).

**P2 - optional**: Normal PD / general Normal CD; ratio solver; engineering symbols and digit separator; UNDO; complex inverse trig; binomial/Poisson lists.

## Test plan summary

For each CORE capability the register must show: normal, boundary, invalid input, accuracy against an independent oracle, key-sequence, and regression coverage. `tools/audit/register.py --check` runs in CI so new lexer words cannot ship without a catalogue row. The approved-token list makes any new token a deliberate, reviewed change.
