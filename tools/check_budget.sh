#!/bin/sh
# Embedded memory budget check (Cortex-M0+ cross-compile for code/static RAM, host stack painting for stack).
# Ceilings come from docs/exam/03-exam-specification-proposed.md section 4. This is NOT a hardware measurement.
# Usage: tools/check_budget.sh [build_dir_with_bench_stack]
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
BUILD=${1:-$ROOT/build}
FLASH_MAX=$((192 * 1024)); RAM_MAX=$((48 * 1024)); STACK_MAX=12288; LCD=1536
OUT=$ROOT/build-fp-thumbv6m-none-eabi
sh "$ROOT/tools/footprint.sh" thumbv6m-none-eabi "$OUT" > "$OUT.log" 2>&1 || { cat "$OUT.log"; exit 1; }
flash=$(sed -n 's/^flash (text+data) = \([0-9]*\) bytes, static RAM (data+bss) = \([0-9]*\).*/\1/p' "$OUT.log")
sram=$(sed -n 's/^flash (text+data) = \([0-9]*\) bytes, static RAM (data+bss) = \([0-9]*\).*/\2/p' "$OUT.log")
CFLAGS="--target=thumbv6m-none-eabi -mcpu=cortex-m0plus -mthumb -Os -std=c99 -ffreestanding -fno-builtin -isystem $ROOT/platform/web/libc -I$ROOT -I$ROOT/core -I$ROOT/core/units"
clang $CFLAGS -c "$ROOT/tests/bench/sizes.c" -o "$OUT.sizes.o"
sz() { llvm-nm -S --defined-only "$OUT.sizes.o" | awk -v n="$1" '$4==n {print $2}' | { read h; printf '%d' "0x$h"; }; }
app=$(sz sz_app_t)
stack=$("$BUILD/bench_stack" 0 | sed -n 's/^worst stack high-water: \([0-9]*\) bytes.*/\1/p')
ram=$((app + sram + stack + LCD))
printf '%-34s %8s %10s\n' item bytes ceiling
printf '%-34s %8d %10d\n' "flash (core, -Os, M0+)" "$flash" "$FLASH_MAX"
printf '%-34s %8d\n' "app_t (calculator state)" "$app"
printf '%-34s %8d\n' "  prog_t (compile buffer)" "$(sz sz_prog_t)"
printf '%-34s %8d\n' "static buffers in core (bss)" "$sram"
printf '%-34s %8d %10d\n' "worst stack (host-painted)" "$stack" "$STACK_MAX"
printf '%-34s %8d\n' "LCD frame buffer (192x64x1bit)" "$LCD"
printf '%-34s %8d %10d\n' "RAM total" "$ram" "$RAM_MAX"
fail=0
[ "$flash" -le "$FLASH_MAX" ] || { echo "FAIL: flash over budget"; fail=1; }
[ "$ram" -le "$RAM_MAX" ] || { echo "FAIL: RAM over budget"; fail=1; }
[ "$stack" -le "$STACK_MAX" ] || { echo "FAIL: stack over budget"; fail=1; }
exit $fail
