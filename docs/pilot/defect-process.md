# Defect logging, prioritising and fixing

1. **Log** every report in the issue tracker (one issue per defect) with: id, reporter role, build id, exact key sequence, expected result and its source (textbook, mpmath, hand calculation), actual result, repeatable?
2. **Reproduce** with the simulator: `build/teleco_sim --text "<keys>"`. If reproducible, add the keys to `tests/ui/keys.txt` or the expression to `tests/cases/*.txt` as a failing test first.
3. **Check the expected value independently** before changing code: the report or the calculator may be wrong. Never take the calculator's own output as the reference.
4. **Prioritise:**
   - **P0 release-blocking**: wrong numeric answer without error, crash/hang/reset, data loss, unsafe memory behaviour. Fix before any further pilot use; notify all testers.
   - **P1**: wrong error/format, feature unusable via keys, confusing result for common school tasks. Fix this pilot cycle.
   - **P2**: cosmetic, rare edge cases, ergonomics. Schedule.
   - **P3**: wishes / new features. Review after the pilot.
5. **Fix** on a branch, keep the regression test, run `tools/ci_local.sh` (gcc, clang, sanitizers, WASM, footprint) and `tools/pilot/run_pack.py`; merge to main only when green.
6. **Record** the fix in the changelog and update `results-record.md` and `known-limitations.md`. Tell the reporter.
