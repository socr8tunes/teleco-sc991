#!/bin/sh
# The same steps the CI workflow runs; usage: tools/ci_local.sh [step...]
# Steps: gcc clang sanitize wasm footprint (default: all)
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
JOBS=${JOBS:-4}
steps=${*:-"gcc clang sanitize wasm footprint"}

build_test() { # name compiler type extra...
  name=$1; cc=$2; type=$3; shift 3
  echo "== $name"
  cmake -S . -B "build-ci-$name" -DCMAKE_C_COMPILER="$cc" -DCMAKE_BUILD_TYPE="$type" "$@" >/dev/null
  cmake --build "build-ci-$name" -j"$JOBS"
  ctest --test-dir "build-ci-$name" --output-on-failure -j"$JOBS"
}

for s in $steps; do
  case $s in
    gcc)       build_test gcc gcc Release ;;
    clang)     build_test clang clang Debug ;;
    sanitize)  build_test san gcc Debug -DTELECO_SANITIZE=ON
               ./build-ci-san/test_ui_fuzz 40 20000 ;;
    wasm)      platform/web/build.sh build-web
               node tests/web/test_wasm.mjs build-web/teleco.wasm
               python3 platform/web/make_html.py build-web/teleco-sc991.html build-web/teleco.wasm ;;
    footprint) tools/footprint.sh thumbv6m-none-eabi build-fp-m0plus | tee build-fp-m0plus.txt
               tools/footprint.sh thumbv7em-none-eabi build-fp-m4 | tee build-fp-m4.txt ;;
    *) echo "unknown step $s"; exit 2 ;;
  esac
done
echo "CI steps passed: $steps"
