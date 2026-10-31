#!/bin/sh
# Build the calculator as a WebAssembly module (clang + wasm-ld, no libc, no emscripten).
# Usage: platform/web/build.sh [outdir]
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${1:-$ROOT/build-web}
mkdir -p "$OUT/obj"
CFLAGS="--target=wasm32 -ffreestanding -fno-builtin -fno-stack-protector -O2 -std=c99 -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes -Werror \
  -isystem $ROOT/platform/web/libc -I$ROOT -I$ROOT/core -I$ROOT/core/units"
OBJS=""
for f in $(find "$ROOT/core" -name '*.c' | grep -v 'core/ui/\(shell\|persist\)\.c' | sort) "$ROOT/platform/web/shim.c" "$ROOT/platform/web/web_api.c"; do
  o="$OUT/obj/$(echo "$f" | sed "s|$ROOT/||; s|/|_|g; s|\.c$|.o|")"
  clang $CFLAGS -c "$f" -o "$o"
  OBJS="$OBJS $o"
done
wasm-ld --no-entry --export-dynamic --allow-undefined-file=/dev/null -z stack-size=1048576 \
  --initial-memory=2097152 --max-memory=16777216 -o "$OUT/teleco.wasm" $OBJS
echo "built $OUT/teleco.wasm ($(wc -c < "$OUT/teleco.wasm") bytes)"
