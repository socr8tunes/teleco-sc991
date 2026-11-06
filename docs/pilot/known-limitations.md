# Known limitations (pilot build)

Be honest with testers about these.

## Not yet verified
- **No real-hardware verification.** All results are from the host build and the PC/WebAssembly simulators. Desktop speed is not evidence of MCU speed (heavy operations such as integrals and sums may be hundreds of times slower on the target; unmeasured).
- **Memory:** about 33 KB static RAM plus 11 KB stack (about 43 KB total), ~123 KB flash (cross-compile size only). Fits an STM32L4 class part (64 KB+); does not fit 12 KB class parts and is marginal at 40 KB. Confirm against the chosen part's datasheet.
- **No exam-body approval or compliance is claimed.** No written confirmation from WAEC or any other examining body exists; do not tell students, schools or customers the calculator is approved or compliant.
- CI workflow is untested on GitHub (all steps pass locally via `tools/ci_local.sh`).
- Constants are CODATA 2022 values taken from scipy.constants; the NIST site could not be fetched here to confirm.

## Behaviour
- Sci n means n significant digits (Casio style); Fix n means n decimals.
- Values within rounding of a power of ten above 9.99999999999999E99 may show as 1E100.
- Complex asin/acos/atan and 10^x of a complex argument are not supported (use e^ or ^ instead).
- Exact special-angle trig can make expressions like 1/sin(pi) report Math ERROR rather than a huge number.
- Cubic equation solver lists roots one at a time starting from the first it finds; press EXE for the next.
- The WebAssembly demo does not keep memory/history between page loads.
- Binomial: n <= 10 000; Poisson: lambda <= 10 000 and x <= 10^9; PFact: 2 <= n <= 9 999 999 999; GCD/LCM/Quot/Rem: integers below 10^16 (LCM errors if the result would exceed 16 digits). Quot and Rem truncate toward zero (Rem(-17,5) = -2).
- InvN(area) is the lower-tail inverse normal; InvN(area, sigma, mu) rescales it.
- Not provided by design (product decision): spreadsheet mode, user-programmable formulas (including TABLE and CALC), macros and scripting, ":" statement chaining.
- Normal distribution Q(t) is the area from 0 to t (Casio convention); R(t) is the upper tail, P(t) the lower tail.
- **Firmware updates discard saved state.** Memory, variables, history, matrices/vectors, statistics data and settings return to defaults after an update that changes the stored format (see `docs/persistence.md`). The first such update is the one adding GCD/LCM/PFact/binomial/Poisson/InvN.
- **Entry limits:** 24 nested bracket levels, 80 number literals and 256 tokens per expression. Past 256 tokens the editor ignores further keys; deeper nesting or more literals gives Stack ERROR on EXE. A number such as 0.5 uses 3 tokens, so the token limit can be reached before 80 literals.
- **Calculus accuracy:** numerical methods; singular integrals and extreme-scale arguments can lose digits (see `tests/known_defects/calculus.txt`). Sums and products are limited to 100001 terms.
