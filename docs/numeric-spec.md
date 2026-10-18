# Teleco numeric representation specification (v0.1)

Status: draft for team review. Implemented in `core/numeric/`.

## 1. Goals

- Decimal-correct results (`0.1 + 0.2` is exactly `0.3`), with no binary `float`/`double` anywhere in the calculation path.
- Fixed size, no heap, portable C99, so the same code runs on the PC simulator and on a Cortex-M0+ class MCU.
- Every basic operation is **correctly rounded**: the result is the representable value nearest to the exact result (error at most 0.5 ulp), so accuracy never depends on the operation order inside a single operation.
- The user sees 10 mantissa digits + a 2-digit exponent. The engine keeps 16, which leaves 6 guard digits for chained operations.

## 2. Representation

```c
typedef struct dec {
    uint64_t m;     /* integer significand, 16 digits: 10^15 <= m <= 10^16 - 1, or 0 */
    int16_t  exp;   /* scientific exponent of the leading digit, -99 .. 99          */
    uint8_t  neg;   /* 1 = negative                                                */
} dec_t;
```

Value = (-1)^neg x m x 10^(exp - 15). Example: 123.45 is `m = 1234500000000000, exp = 2`.

Decisions:

| Question | Decision |
|---|---|
| Do the 16 digits include trailing zeros? | Yes. The form is always normalised to exactly 16 digits (`1` is `m = 10^15, exp = 0`). |
| Can one value have two encodings? | No. The form is canonical, so equality is field equality. |
| Negative zero? | Not representable. Zero is `{0, 0, 0}`. |
| Significand type | Binary integer (not BCD): compact, fast on 32-bit cores, simple rounding. |
| Range | 1e-99 <= abs(x) <= 9.999999999999999e99, plus exact zero. |
| Size | 16 bytes today (padding). It could pack into 8 bytes (54-bit significand + 8-bit exponent + sign) if RAM ever matters. |

## 3. Rounding

Round-half-to-even to 16 significant digits, applied once per operation on the exact result.
`add`, `sub`, `mul`, `div`, `sqrt` and string parsing are all correctly rounded. This is verified against Python's `decimal` module (see section 7).

Internally each operation produces an exact (or exact-plus-sticky) 128-bit integer result, then a single shared routine (`dec_pack_internal`) normalises, rounds and range-checks it.

## 4. Exceptional conditions

Every function returns a `dec_status`. On **any** non-OK status the result is set to zero.

| Status | Condition | Severity |
|---|---|---|
| `DEC_OK` | Normal result. | none |
| `DEC_UNDERFLOW` | Rounded result below 1e-99 in magnitude; flushed to zero. | non-fatal (the calculator layer may treat it as 0) |
| `DEC_OVERFLOW` | Rounded result at or above 1e100 in magnitude. | fatal (Math ERROR) |
| `DEC_DIV_BY_ZERO` | Non-zero divided by zero. | fatal |
| `DEC_DOMAIN` | `0/0`, `sqrt` of a negative number. | fatal |
| `DEC_PARSE` | Malformed input string. | fatal |

The range check happens **after** rounding, so a value such as 9.9999999999999995e99 that rounds up to 1e100 overflows.

## 5. Intermediate results (open item)

Each operation is range-checked on its own. An expression like `a * b / c` can therefore overflow in `a * b` even when the final answer would fit. This matches the behaviour of most calculators and is simple to reason about. If the team wants a wider intermediate range, the evaluator can later use an extended-exponent type internally. This is a product decision, not a numeric-core limitation.

## 6. Strings

- Output (`dec_to_str`): `-d.dddddddddddddddE+XX` (16 significant digits, 2-digit exponent), or `0`. This is the canonical *engineering/test* format. User-facing formatting (Fix/Sci/Norm/Eng, 10 digits) is a separate layer built on top.
- Input (`dec_from_str`): `[+-]digits[.digits][(e|E)[+-]digits]`. Inputs with more than 16 digits are rounded correctly (up to 34 significant digits are held exactly, anything beyond acts as a sticky bit).

## 7. Verification strategy

1. Hand-written known-value unit tests (`tests/unit`).
2. Differential tests (`tests/differential`): a Python generator builds reference vectors with `decimal.Context(prec=16, rounding=ROUND_HALF_EVEN)`, and a C runner executes them. The generator deliberately targets ties, exponent alignment, catastrophic cancellation, overflow/underflow boundaries, perfect squares and long input strings. Failures print the exact line, and the seed reproduces the run.
3. Pinned regression vectors in `tests/regression`.
4. Sanitizer (ASan + UBSan) build option.

## 8. Not covered yet

- **Transcendental functions** (exp, ln, trig, ...) need more than 16 digits internally to deliver results correct to 16. The plan is an extended-precision working type (about 24 to 32 digits, built on the same 128-bit integer helpers) used only inside those functions.
- **Exact types** (fractions, surds, pi multiples, complex) live in `core/values` as a tagged union on top of `dec_t`.
- **MCU benchmarking.** PC timings are only a smoke test. The cost drivers on a Cortex-M0+ will be 64-bit division (software) and the 128-bit square root loop. Measure on the STM32U0 dev board before locking the design.
