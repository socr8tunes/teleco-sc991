# 05 - Technical risks and recommended implementation priorities

Severity: H = could block release or mislead customers; M = significant rework or quality risk; L = manageable.

## Risks

| # | Risk | Sev | Evidence | Recommended action |
|---|---|---|---|---|
| R1 | **Exam acceptance unknown.** No WAEC technical specification was found; Casio-parity does not guarantee acceptance (CALC, TABLE, matrix, complex, base-n, replay, persistent memory could each be questioned). | H | Public searches found no WAEC calculator spec | Contact WAEC/Ghana Education Service now with the section 1-2 boundary list; ask for written criteria; do not claim approval |
| R2 | **"417 functions" is unsupported.** The repo never defines it; it is Casio's marketing number for the ES PLUS. We can only count 295 reachable entries under our own rule. A numeric claim equal to a competitor's figure invites comparative-advertising and trade-description problems. | H | Casio pages; `grep 417` | Drop the number; publish a capability list or our own counting rule |
| R3 | **Entry capacity is weak.** 11 bracket levels (docs claim 24) and 40 number literals per expression; a 41-term sum fails with Stack ERROR. | H | `limits-probe.md`; `MAX_CONSTS 40`, expr depth accounting | Decide targets (spec section 3); raise or dedupe literals; extend tests; fix documentation now |
| R4 | **No hardware evidence.** Host speed is not MCU speed; integrals/sums/Solve allow up to 60,000 evaluations; Time Out semantics unverified. | H | known-limitations.md | Bring up a Cortex-M4 board; measure worst-case latency and stack; set Time Out by time, not count |
| R5 | **Memory is larger than documented.** About 40 KB RAM and 125 KB flash for the core alone (docs: "~30 KB plus stack, 117 KB"). Rules out 128 KB-flash and 40 KB-RAM parts. | H | sizeof app_t 16,112; static 12,304; stack 10,468; flash 124,556 | Choose an MCU with 256 KB flash and at least 64 KB RAM; correct docs; track budget in CI (stack budget test exists, flash/RAM budget does not) |
| R6 | **Test depth is uneven.** 82 of 131 catalogued capabilities lack required evidence; calculus and statistics have zero error-case tests; matrices, vectors, complex, base-n and equations are hand-verified only (23-line equations file); no nested-bracket or token-limit tests; only one Time Out test. | M | Register | Execute the test plan in priority P0 below |
| R7 | **Documentation overstates.** `docs/feature-gap-audit.md` cites `tests/unit/test_exclusions.c`, which does not exist; the guard is two sample cases. Bracket limit stated as 24. Feature audit still lists TABLE/CALC as excluded although restoration was approved. | M | repo | Correct docs; write the real guard test (spec 1.6) |
| R8 | **Non-programmability must be demonstrable.** CALC and TABLE hold an expression for re-evaluation; the replay history persists past expressions. A reviewer may judge these "stored formulas". | M | design | Keep them session-only and single-expression; demonstrate by test; explain in the WAEC submission |
| R9 | **Saved-state format churn.** Every inserted token shifts codes and forces a persistence version bump that wipes user data. Acceptable pre-release; unacceptable for shipped firmware updates. | M | VERSION now 6 | Freeze the token enum with reserved gaps (or stable IDs in persistence) before first customer release |
| R10 | **Reference data partly from memory ("?").** Casio availability of GCD/LCM/Int÷/Rmdr, ES PLUS matrix/stat limits, EX constants count. | M | 02-casio-gap-analysis | Verify against official guides before comparing publicly |
| R11 | **Quartic engine is unverified.** Written, compiles, not tested; uses a heuristic exact-reconstruction stage; iteration cap and ill-conditioned cases untested. | M | branch wip/quartic-parked | Do not merge until test plan in the original requirements is met (mpmath, repeated/near-repeated, scaling, 1000 random) |
| R12 | **Look-alike layout.** The key layout and legends closely follow Casio's ES layout. | M | docs/keymap.md | Legal review of trade-dress exposure; consider layout differentiation |
| R13 | **Syllabus fit untested.** No review against WAEC SHS core/elective mathematics, physics, chemistry syllabi or Ghanaian university first-year content. | M | pilot questions are author-written | Map syllabus topics to capabilities; add teacher-reviewed questions |
| R14 | **Firmware extraction/programming port.** A "non-programmable" claim requires a locked, sealed firmware path in hardware. | M | not designed | Add to hardware requirements (readout protection, no user-accessible bootloader) |

## Recommended priorities

**P0 - before any new feature (this audit's follow-ups; no new functionality)**
1. Correct the documentation errors in R5, R7 (bracket limit, memory figures, missing guard test, audit table).
2. Verify the "?" Casio items (R10) and WAEC contact (R1).
3. Decide and record entry-capacity targets (R3); then fix as a defect with boundary tests.
4. Close test debt for CORE capabilities: error and boundary cases for calculus, statistics, matrices, vectors, base-n, complex, equations, trig domain; independent oracles for matrix/vector/complex/base-n (Python Fractions/mpmath); key-path tests for every menu item; committed limits tests from `limits-probe.md`.
5. Write the exclusions guard test (spec 1.6).

**P1 - core feature gaps (after P0 review)**
1. Restore CALC and TABLE per approved rules.
2. Finish quartic from the parked branch with the approved verification plan.
3. Inequality solver (reuses the polynomial engine).
4. Hardware bring-up and timing (can run in parallel).

**P2 - optional**: Normal PD / general Normal CD; ratio solver; engineering symbols and digit separator; UNDO; complex inverse trig; binomial/Poisson lists.

## Test plan summary (to be executed after approval)

For each CORE capability the register must show: normal, boundary, invalid input, accuracy against an independent oracle, key-sequence, and regression coverage. The register generator (`tools/audit/register.py --check`) is run in CI so new lexer words cannot ship without a catalogue row; the pilot pack is extended with independently computed answers (mpmath/Fractions) for every capability that currently has none, and a second reviewer re-derives a sample of answers by hand.
