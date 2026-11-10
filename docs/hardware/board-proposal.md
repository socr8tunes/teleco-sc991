# SC991 Exam: bring-up board proposal (for approval, nothing purchased)

Status: PROPOSED 2026-11-10. Follows the approved Cortex-M4 bring-up plan (`cortex-m4-bringup-plan.md`, decision D10: at least 256 KB flash and 64 KB RAM). No purchase is made until you approve a board.

## 1. Proposal

**Primary: ST NUCLEO-L452RE** (STM32L452RE, Cortex-M4F 80 MHz, 64-pin, ST-LINK/V2-1 on board).

| Item | Value | Source / status |
|---|---|---|
| Flash | up to 512 KB, single bank, 256 pages of 2 KB | ST product page and datasheet (read 2026-11-10) |
| SRAM | 160 KB (128 KB SRAM1 + 32 KB SRAM2 with parity) | same |
| Stop 2 with RTC | 2.4 uA (features list; a second table in the datasheet says 2.30 uA: confirm in the electrical tables) | ST product page / datasheet |
| Flash endurance | **not verified**: the pages read did not state it. ST confirms the figure counts page erases, not writes. Must be read from the datasheet table "Flash memory endurance and data retention" before H3 | open |
| Trace | ETM supported; SWO and DWT cycle counter are standard Cortex-M4 features, to be confirmed on the board | to confirm at H1 |
| Price | **AUD 29.20 ex GST (AUD 32.12 incl. GST)**, 183 in stock (element14 Australia, read 2026-11-10); the same listing shows a 31-week manufacturer lead time for new stock. Prices at other distributors vary; no African distributor price was found | retailer listing |

Why this part and not the exact minimum: the 512 KB / 160 KB part lets the **linker script be limited to 256 KB flash and 64 KB RAM**, so the build must still fit the stated minimum while the extra memory is available for measuring (stack painting beyond the limit shows the true demand). The gate "fits in 256 KB / 64 KB" is then enforced by the linker, not assumed.

**Optional second board: NUCLEO-L432KC** (STM32L432KC, 256 KB flash, 64 KB RAM, Nucleo-32), NZD 28.35 ex GST at element14 NZ (read 2026-11-10). This is the exact-fit part (flash and RAM sizes are from my knowledge of the part, not from the page read: confirm on the datasheet). Buy it only after H1 to H5 pass on the L452RE, as the final check that the minimum part really suffices; its 32-pin package has few free pins for the key matrix and display, so it is not suitable as the main bring-up board.

**Alternative considered: NUCLEO-L476RG** (1 MB, 128 KB, EUR 23.50 at botland, business customers only). More memory than needed and no closer to the target; rejected as primary.

## 2. Other items needed for H0 to H2 (to be quoted, no prices found yet)

| Item | Note |
|---|---|
| 192x64 monochrome graphic LCD module on SPI (ST7565 / UC1701 class) | confirm controller and 3.3 V logic; the renderer already produces a 192x64 1-bit frame buffer |
| Key matrix | breadboard with 6 x 9 tact switches or a spare keypad; diodes for the ghost-key rule |
| USB cable, jumper wires, breadboard | trivial |
| Optional: current-measurement shunt or a power profiler | needed only for the power measurement in section 3 |

## 3. Expected bring-up measurements

Already defined in plan section 4; restated with what is expected on this part. Host figures are the baseline.

| Measurement | Host / current figure | Expected on the L452RE | Gate |
|---|---|---|---|
| Flash used by the core (-Os, M4) | 129,883 B | about the same plus HAL (tens of KB at most) | at most 75% of the 256 KB limit |
| Static RAM | 16,180 B core, 45,596 B total budget | same core; HAL adds about 2 KB | at most 80% of 64 KB |
| Worst-case stack, host-painted | 11,512 B of 12,288 B (6% headroom, R15) | may differ with the Thumb-2 compiler; **expected to be the figure most likely to change** | at least 25% headroom of the allocation; the allocation is chosen from the measurement |
| Time, ordinary arithmetic, trig | host microseconds (bench_calc) | to be measured with the DWT counter | at most 2 s per operation |
| Time, worst Solve (no root), worst integral (two passes, D15), 100,001-term sum | host about 70 ms for Solve | to be measured; the integral now does two passes | at most 2 s or bounded by the time-based Time Out |
| VM step rate | not applicable | steps per second on target | converts the 4,000,000 step budget into seconds |
| Storage, power cut | not applicable on host | 1,000 injected power cuts, never a mixed blob | valid every time; 10 years at 20 saves per day |
| Power | not applicable | run, Stop 2 and OFF current | solar plus coin-cell budget (hardware stage) |

## 4. Solve time-out (D17)

The time-based Solve Time Out is introduced during bring-up as a **configurable value**:

* a single constant `TELECO_SOLVE_TIMEOUT_MS` in the HAL timebase header, default 5,000 ms, overridable at build time and by a debug command on the board during bring-up only;
* the solver polls the timebase every N function evaluations (N chosen so the check costs under 1%);
* on expiry the result is Time Out (not a number), and the existing evaluation budget remains as a backstop;
* the final value is set from the measured worst case at H5 and then frozen in the specification; until then it is provisional.

## 5. Acceptance gates for the board stage

1. H1: on-target `size` equals `tools/footprint.sh` for the same build.
2. H2: every key in `docs/keymap.md` reaches the shell; the frame buffer appears on the panel unchanged.
3. H3: storage tests pass on real flash, with 1,000 injected power cuts.
4. H4/H5: measurement table committed to `docs/hardware/measurements.md`; stack, RAM, flash and time gates above met; risks R4, R5, R15 updated from measurements.
5. H6: 48-hour soak without fault.
6. Linker limit of 256 KB flash and 64 KB RAM holds for the whole build.

If a gate fails, the response order stays as in the plan: reduce stack in the deepest frames, then reduce capacity limits only with your approval, then raise the MCU class.

## 6. Decision requested (D19)

Approve the NUCLEO-L452RE as the primary bring-up board (about AUD 29 plus a display module and key matrix parts to be quoted), with the L432KC held back as the final exact-fit check. Please also say whether the board should be bought from a distributor that ships to you in Ghana; I found no local price, so the cost to you may include shipping and import duty that I cannot estimate.

Sources: [element14 Australia, NUCLEO-L452RE](https://au.element14.com/stmicroelectronics/nucleo-l452re/dev-board-cortex-m4-mcu/dp/2677824); [element14 NZ, NUCLEO-L432KC](https://nz.element14.com/stmicroelectronics/nucleo-l432kc/dev-board-nucleo-32/dp/2580786); [Botland, NUCLEO-L476RG](https://botland.store/stm32-nucleo/18800-stm32-nucleo-l476rg-with-stm32l476rgt6-arm-cortex-m4-mcu-5904422364977.html); [ST, STM32L452RE](https://www.st.com/en/microcontrollers-microprocessors/stm32l452re.html); [STM32L452xx datasheet](https://www.digikey.at/htmldatasheets/production/2305652/0/0/1/stm32l452xx.html); [ST community, flash endurance meaning](https://community.st.com/t5/stm32-mcus-products/flash-endurance-parameter/m-p/637907).
