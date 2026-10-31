#!/bin/sh
# Cross-compile the portable core for Cortex-M (clang, no linker) and report flash / RAM per module.
# This measures code size and static RAM only. It is NOT proof of hardware readiness (see docs/hardware-readiness.md).
# Usage: tools/footprint.sh [thumbv6m-none-eabi | thumbv7em-none-eabi] [outdir]
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
TARGET=${1:-thumbv6m-none-eabi}
OUT=${2:-$ROOT/build-fp-$TARGET}
CPU=cortex-m0plus; [ "$TARGET" = thumbv7em-none-eabi ] && CPU=cortex-m4
mkdir -p "$OUT"
CFLAGS="--target=$TARGET -mcpu=$CPU -mthumb -Os -std=c99 -ffreestanding -fno-builtin -fno-stack-protector -ffunction-sections -fdata-sections \
  -fstack-size-section -isystem $ROOT/platform/web/libc -I$ROOT -I$ROOT/core -I$ROOT/core/units"
for f in $(find "$ROOT/core" -name '*.c' | sort); do
  clang $CFLAGS -c "$f" -o "$OUT/$(basename "$f" .c).o"
done
printf '%-14s %8s %8s %8s\n' module text data bss
tt=0; td=0; tb=0
for o in "$OUT"/*.o; do
  set -- $(llvm-size -B "$o" | tail -1)
  printf '%-14s %8s %8s %8s\n' "$(basename "$o" .o)" "$1" "$2" "$3"
  tt=$((tt + $1)); td=$((td + $2)); tb=$((tb + $3))
done
printf '%-14s %8s %8s %8s\n' TOTAL $tt $td $tb
echo "flash (text+data) = $((tt + td)) bytes, static RAM (data+bss) = $((td + tb)) bytes (excludes compiler-rt helpers and the call stack)"
