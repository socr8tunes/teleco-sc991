# 07 - Remaining decisions for approval

Nothing below has been started. No CALC, TABLE, inequality or quartic work has been done; those stay on hold until you approve.

| # | Decision | Options and recommendation |
|---|---|---|
| D1 | **WAEC enquiry.** Send a written enquiry with the boundary list (spec sections 1-2) and ask for written criteria. | Recommended: yes, now, before more features are built. No approval or compliance wording is used anywhere until written confirmation arrives. |
| D2 | **Release of the hold** on CALC and TABLE (approved rules), then inequality solver, then quartic. | Recommended order: CALC, TABLE, inequality, quartic (quartic last, with the approved verification plan). Confirm or reorder. |
| D3 | **":" multi-statement entry.** Both Casio guides have it. | Recommended: keep removed (simpler non-programmability argument); revisit only if WAEC asks. |
| D4 | **Singular and non-convergent integrals.** No error-estimate threshold separates good from bad results (data in the risk notes). | (a) leave and document (recommended for the pilot); (b) conservative non-convergence error, accepting some false errors on valid integrals. |
| D5 | **Absolute step and tolerance on tiny arguments** (`ddx(X^2,1E-50)` = 0; `solve(X^2=1E-20,1)` stops at 5.07E-10) and the bogus root of `solve(e^X=0)`. | (a) document only; (b) make step and tolerance scale-relative and require a relative residual (recommended; small change plus tests). |
| D6 | **Optional features** (Normal PD / general CD, ratio solver, engineering symbols, digit separator, UNDO, complex inverse trig). | Recommended: after P1; UNDO and engineering symbols are EX-only, so defer. |
| D7 | **Register rule.** Strict rule demands an oracle even for UI features. | (a) relax for display/settings rows (recommended; about 15 rows); (b) keep and build an image-level oracle for the display. Either way the remaining ~37 rows are closed by more cases after your review. |
| D8 | **Persistence stability.** Freeze token IDs (reserved gaps) before the first customer release. | Recommended: freeze before pilot hardware is handed out. |
| D9 | **Stack headroom 13%** (goal 25%). | (a) accept until hardware measurement (recommended); (b) reduce frames or raise the ceiling now. |
| D10 | **MCU class.** At least 64 KB RAM and 256 KB flash from the measured budget. | Confirm, and approve Cortex-M4 bring-up to measure latency and stack. |
| D11 | **GCD, LCM, Quot, Rem** are not in either Casio guide. | Keep as extras (recommended), but never describe them as Casio parity. |
| D12 | **Layout look-alike** (R12). | Commission a legal review of the key layout and legends. |
