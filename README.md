# Teleco scientific calculator

Portable C99 calculator engine, built and tested on a PC first and then run on a low-power MCU
(solar + coin cell, dot-matrix LCD, natural textbook display).

## Status

Milestone 1: decimal number core (`core/numeric`), spec, test harness. See `docs/numeric-spec.md`.

## Build and test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Useful options:

```sh
cmake -S . -B build-san -DTELECO_SANITIZE=ON          # ASan + UBSan
cmake -S . -B build -DTELECO_DIFF_COUNT=200000        # more differential vectors
cmake -S . -B build -DTELECO_DIFF_SEED=42             # different random seed
./build/bench_decimal                                 # micro-benchmark
```

Differential tests need Python 3 (standard library only).

## Layout

```
core/       portable engine, no hardware or OS dependencies
hal/        hardware abstraction interfaces (keypad, display, power, storage)
platform/   pc (simulator) and stm32 (firmware) implementations of the HAL
tests/      unit, differential (Python decimal reference), regression, bench
docs/       specifications
```

## Rules

- `core/` never includes STM32 headers, LCD drivers or OS APIs, and never uses `float`/`double` in calculations.
- No heap allocation in `core/`.
- Any bug found by a test gets a pinned vector in `tests/regression/`.
