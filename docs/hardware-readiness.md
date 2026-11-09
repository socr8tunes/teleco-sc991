# Hardware readiness - Teleco SC991

**Status: not verified on hardware.** Everything below the line "Measured on the host / by cross-compiling"
was measured without a target board. Nothing in this repository proves that the firmware runs on real silicon,
that the display or key matrix work, or that battery/solar operation meets the product goal. Desktop and WebAssembly
results (tests, timings) are **not** evidence of hardware readiness. The checklist at the end is what has to be
done on a prototype before a pilot can rely on the hardware.

## What exists

| Piece | State |
|---|---|
| Portable C99 core (numeric, parser, editor, renderer, UI, shell loop) | complete, tested on host, sanitizers, wasm |
| HAL contract (`hal/display.h`, `keypad.h`, `power.h`, `storage.h`, `timebase.h`) | defined; PC implementation (`platform/pc/hal_pc.c`) used by tests |
| Main loop `shell_step` (key edge detection, 2-key rollover, render on change, sleep when idle, auto power-off, state save) | complete, tested with the PC HAL and a fake clock |
| Persistence (`persist.c`): magic/version/size/CRC32, streamed, previous blob survives an interrupted save | complete, tested (corruption, interrupted commit) |
| `platform/stm32/main.c` | entry point only (compiles for Cortex-M0+) |
| `platform/stm32/board_template.c` | stub HAL satisfying the contract (links, drives no hardware); compile-checked by `tools/check_template.sh` |
| STM32 HAL (display driver, key matrix scan, STOP-mode sleep, retained storage, RTC tick) | **not written** - needs the board and the panel datasheet |

## Measured on the host / by cross-compiling (clang, `-Os`, no linker; `tools/footprint.sh`)

| Item | Cortex-M0+ | Cortex-M4 |
|---|---|---|
| Code + constants (flash) | 126.6 KB (129,659 B) | 126.8 KB (129,883 B) M4 build |
| Static RAM in core (`.bss`/`.data`: VM and solver state) | 14.8 KB (15,152 B) | not re-measured |

Not included in those numbers: compiler runtime helpers (64-bit division etc., a few KB), the startup code, a flash
driver, and the HAL.

Other RAM, from `sizeof`:

| Object | Size |
|---|---|
| `app_t` (calculator state incl. `ctx_t`, history 1 KB, editor 0.5 KB) | 16.0 KB (16,368 B) |
| `lcd_t` framebuffer 192x64 | 1.5 KB |
| **Static total** (core bss + `app_t` + framebuffer) | **about 33 KB** |
| Largest stack frames: `nm_derivative` 2.3 KB, `stat_var` 2.2 KB, `mx_inv` 2.1 KB, `nm_solve` 1.7 KB, `parse_fn` 1.6 KB, `ui_logo_render` 1.6 KB | stack depth is **not** yet measured |

Consequences:

* **RAM**: static state plus stack is about 43 KB. A 12 KB part (STM32U031-class) cannot hold it; a 40 KB part
  (STM32U073/U083-class) is marginal; a 64 KB part (STM32L4 family, for example L432) fits. Confirm RAM sizes against
  the datasheets before choosing. Reduction levers, largest first: fewer temporary matrices (4 -> 2, about -3.6 KB),
  smaller matrix element type, a narrower `val_t`, sharing the VM/solver scratch.
* **Flash**: 126 KB plus runtime fits a 256 KB part with room for a second retained-state page.
* **Speed**: host timings (`tests/bench/bench_calc.c`, desktop CPU): typical expression 1-15 us; `ddx` 56 us;
  `solve` 54-84 us; 3x3 inverse 43 us; `integral(e^(-X^2),0,3)` 1.4 ms; a 1000-term `sum` 2.9 ms. A 48 MHz Cortex-M0+
  without 64-bit hardware multiply/divide is plausibly 200-1000x slower per operation (rough guess, **to be measured**):
  that would put a 1000-term sum at roughly 0.6-3 s. The VM step budget (`VM_BUDGET`, 4,000,000 steps, then
  "Time Out") has to be calibrated against wall-clock time on the target.

## Display

The renderer produces a 192x64, 1 bit-per-pixel frame (`hal_display_present`). At an 8 MHz SPI the 1,536-byte frame
takes about 1.5 ms. The panel controller is not chosen; the HAL needs: init sequence, contrast (`hal_display_set_contrast`, 0-63),
power on/off, and a frame push. Fonts are original to this project (`tools/gen_font.py`), 5x7 and 3x5.

## Keypad

49 logical keys (`core/ui/keys.h`), a 7x7 diode-isolated matrix is sufficient. `hal_keypad_scan` must return the
debounced set of held keys (up to 2); `shell_step` does press detection. `hal_keypad_arm_wake` must arm the
wake-from-STOP interrupt on every column.

## Power

Goal (from the brief): CR2032 plus solar. Not measured. Firmware behaviour that supports it: no polling when idle
(`hal_power_sleep`, 250 ms only for cursor blink, unbounded wait when off), auto power-off after 10 minutes, state saved at
power-off. The blink wake-up every 250 ms costs current; consider a blink-free cursor to sleep until the next key.

## Hardware verification checklist (all open)

| ID | Check | Pass criterion |
|---|---|---|
| HW-1 | Every key reads correctly, 2-key rollover, no ghosting, no bounce double-presses | 1,000 presses per key without a miss or double |
| HW-2 | Display init, contrast range, no flicker, frame push time | readable 0-63 contrast, <= 5 ms push |
| HW-3 | Stack depth (stack painting) over the worst expressions (nested solve/integral/sum, 4x4 matrix inverse, regression) | >= 25% headroom |
| HW-4 | Flash image and RAM map from the real linker | fits with the chosen MCU, headroom stated |
| HW-5 | Key-to-display latency | simple expression <= 100 ms; worst built-in function <= 10 s with a visible indicator, then a clean "Time Out" |
| HW-6 | `VM_BUDGET` calibration | maps to a wall-clock limit chosen by the team |
| HW-7 | Sleep current, active current, CR2032 life, solar-only operation at a stated lux | numbers recorded; goal set by the team |
| HW-8 | Brown-out / battery removal: retained state and interrupted save | last good state survives pulling the battery during a save |
| HW-9 | Battery-low threshold and behaviour | warning shown before calculation errors can occur |
| HW-10 | Long-run soak with random key input on the device (the host fuzz test, driven over a debug UART) | no hang, no corruption for 24 h |

## Host stack measurement (`bench_stack`, ctest `stack_budget`)

Stack painting on a 64-bit host over the heaviest expressions (nested integral, cubic, solve, matrix inverse, deep sums):
**worst high-water 10.7 KB** (nested integral 10.7 KB; 24-level nested brackets 10.7 KB). 64-bit frames are usually not smaller than Cortex-M
frames, so plan about 11-12 KB of stack plus UI render depth; the test fails above 12 KB. This is a host estimate only;
HW-3 must repeat it on the device. With ~33 KB static RAM this means **about 42.7 KB RAM total** (`tools/check_budget.sh`: 45,596 B against a 48 KB ceiling), so a 64 KB part
(STM32L4 class) fits and a 40 KB part does not. These are cross-compile and host-painting figures, not hardware measurements.

## Bring-up plan

1. Pick the dev board and panel; write `board_init`, the timebase and a display "hello" using the font table.
2. Key matrix scan and a key-echo screen.
3. Link the shell loop: `shell_init`/`shell_step` with the real HAL; run `tests/ui/keys.txt` through a UART key injector and compare results with the host.
4. Measure HW-3..HW-6 and tune the budget; then power work HW-7..HW-9.
5. Only then plan the pilot hardware lot.

## Exam approval

No approval by, or compliance claim towards, WAEC or any examinations body exists. None may be made without written confirmation from that body. It typically concerns non-programmable, non-graphing,
no text storage/communication and an approved model list; confirm the current rules with the examining body and
record the answer here before the pilot is described as exam-ready.
