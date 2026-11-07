# SC991 Exam - Cortex-M4 bring-up plan (draft for approval)

Status: planning only; no hardware has been used yet. Date: 2026-11-07. Scope: the SC991 Exam firmware. Nothing in this plan adds Pro or AI functionality.

## 1. Purpose

Replace host-based estimates with measurements. The open questions it must answer are the stack margin (provisionally 13%, risk R15), the real time cost of the heaviest operations (R4), the RAM and flash totals (R5), and whether saved state survives power loss on real flash (R9). Until these are measured, every stack and timing figure in the documents stays "provisional".

## 2. Target and selection criteria (decision D10, approved)

| Requirement | Minimum | Why |
|---|---|---|
| Core | Cortex-M4 (or M33) with FPU not required (the numeric core is integer decimal) | measured footprint is built for thumbv7em; M0+ is the fallback to check |
| Flash | at least 256 KB (core 125 KB today) | leaves room for HAL, CALC, TABLE, inequality, quartic and a second storage page |
| RAM | at least 64 KB (core 43.7 KB static + 10.7 KB host-measured stack) | headroom for HAL buffers and the stack margin |
| Low power | STOP-class mode with RTC wake; current sufficient for solar + coin cell | auto-off and retained state |
| Flash endurance | at least 10,000 erase cycles per page, page-erase granularity known | blob rewrite on every power-off |
| Debug | SWD with SWO/ITM and DWT cycle counter | timing and stack measurement |
| Security | readout protection available (risk R14) | firmware extraction |

Candidate families to compare against their datasheets before purchase (figures not verified here): the 256 KB / 64 KB members of the ultra-low-power STM32L4 line, and equivalent parts from other vendors. A development board with the chosen MCU and an external 192x64 monochrome panel on SPI is enough; no custom PCB is needed for this phase.

## 3. Phases and exit gates

| Phase | Work | Exit gate |
|---|---|---|
| H0 | Select MCU and development board; confirm the numbers in section 2 from the datasheet; order a 192x64 monochrome panel and a key matrix breadboard | decision record signed off |
| H1 | Toolchain, startup file, linker script with the core's static RAM placed explicitly; blink and UART/SWO log | `tools/footprint.sh` output equals the on-target `size` output |
| H2 | HAL: `timebase` (SysTick/RTC), `display` (SPI panel, contrast 0-63), `keypad` (matrix scan, debounce, ghost-key rule), `power` (STOP, wake key) | every key in `docs/keymap.md` reaches the shell; display shows the renderer's frame buffer unchanged |
| H3 | HAL `storage`: two-page flash commit with CRC and sequence number, power-fail safe; wear estimate | `tests/unit/test_shell.c` storage cases pass against the real flash driver (corruption, torn write, version discard) plus 1,000 power-cut cycles injected by cutting supply during a save |
| H4 | Measurement harness (section 4) running the bench set over UART/SWO | table of results committed to `docs/hardware/measurements.md` |
| H5 | Set limits from measurements (section 5), re-run, update risk register R4, R5, R15 | gates below |
| H6 | 48-hour soak: scripted key presses, random expressions from the pilot bank, auto-off cycles, brown-out | no hard fault, no state corruption, storage still valid |

## 4. Measurements

All results are recorded with MCU part number, clock, compiler version and flags, and the git commit.

1. **Stack high-water.** Paint the stack with a known pattern at start-up, run each worst-case expression of `tests/bench/bench_stack.c` (24 nested brackets, 24 unary minus, 24 nested sin, 80-term sums, nested integrals, cubic, Simul4, matrix 4x4) and record the painted depth after each. Report the maximum and the margin against the stack allocated.
2. **Static RAM and flash.** From the linker map: `.bss`, `.data`, `.rodata`, `.text`. Compare with `tools/check_budget.sh`.
3. **Time per operation** with the DWT cycle counter, median and worst of 20 runs: basic arithmetic, trig, ln, e^x, x^y, factorial 69, nCr, `sum` of 10 / 1,000 / 100,001 terms, `prod`, integral (well-behaved and the 80-interval worst case), derivative, Solve (converging, no-root worst case, which tries up to about 1,040 + 4,800 function evaluations), quadratic, cubic, Simul4, matrix 4x4 inverse and determinant, 80-row regression, and a 256-token expression.
4. **Virtual-machine step rate.** Steps per second on the device, to convert the current budget (4,000,000 VM steps per EXE) into seconds.
5. **Power.** Current in run, STOP and OFF; wake-up time.
6. **Storage.** Time to save, bytes written, pages erased per power-off, result of power-cut injection.

## 5. Acceptance gates (proposed; to be confirmed with the measurements)

| Item | Gate |
|---|---|
| Stack | measured worst-case high-water leaves at least 25% of the stack allocated (the host figure is 10.7 KB; the allocation on target is chosen from this, not the reverse) |
| RAM total | at most 80% of the chip RAM including HAL |
| Flash | at most 75% of the chip flash including HAL and a second storage page |
| Time | every CORE operation finishes in at most 2 s at the production clock; operations that cannot are bounded by a time-based Time Out (default 5 s, to be confirmed) instead of the VM step budget |
| Key response | key to screen update at most 100 ms for ordinary entry |
| Storage | 1,000 injected power cuts leave a valid blob every time (old or new, never a mix); wear leaves at least 10 years at 20 saves per day |
| Soak | 48 h without fault |

If a gate fails the response is chosen in this order: reduce stack use in the deepest frames (solver and parser), reduce capacity limits only with your approval, raise the MCU class.

## 6. Software changes expected

- `hal_*` implementations for the chosen board under `platform/stm32/` (the main loop and the template exist; no board code does).
- Replace the step-count Time Out with a time-based limit using the timebase (keeps the step budget as a second guard).
- A small on-target test runner that executes the `tests/cases` files from flash and prints failures (the case files are plain text; use a trimmed subset that fits).
- Stack-painting and cycle-counter helpers behind a build flag (not in the release image).

## 7. Risks and dependencies

| Risk | Mitigation |
|---|---|
| Decimal arithmetic much slower than estimated, so Σ, ∫ and Solve exceed 2 s | measure early (H4 starts with the step rate); the time-based Time Out is the safety net; algorithm limits (100001 terms, 80 intervals) can be lowered with approval |
| Stack margin below the gate | frame reduction in the parser and numerical methods before any capacity reduction |
| Flash endurance or erase time makes saving at power-off unreliable | write only on change, rotate two pages, keep the CRC and sequence number scheme |
| Display or key matrix supplier changes | HAL isolates both; renderer works on a 1-bit frame buffer |
| Readout protection not available on the chosen part | treated as a selection criterion in H0 |

## 8. Decisions needed from you before H0 closes

1. Approve the MCU criteria in section 2 (or change the minimums).
2. Approve buying a development board and a panel (cost to be quoted at H0).
3. Approve the proposed gates in section 5, in particular the 25% stack margin and the 2 s operation limit.
