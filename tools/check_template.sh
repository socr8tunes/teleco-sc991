#!/bin/sh
# Syntax/warning check of the Cortex-M board template and entry point against the HAL contract.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
for f in board_template.c main.c; do
  clang --target=thumbv7em-none-eabi -mcpu=cortex-m4 -mthumb -std=c99 -ffreestanding -Wall -Wextra -Wpedantic -Werror \
    -isystem "$ROOT/platform/web/libc" -I"$ROOT" -I"$ROOT/core" -I"$ROOT/core/units" -c "$ROOT/platform/stm32/$f" -o /dev/null
done
echo "stm32 template compiles"
