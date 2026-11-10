# CALC: specification and verification plan (for review, no implementation yet)

Status: **PROPOSED, 2026-11-09.** Author: engineering. Scope: SC991 **Exam** model only. Nothing in this document is implemented. Implementation starts only after the owner approves this document (decision D18, section 10).

Rules already approved (D2): CALC is a **single expression**, **session only**, **never persisted**. No approval or compliance claim is made for any examination body; CALC is one of the features most likely to be questioned (risk R1), so the build can switch it off (section 8).

## 1. What CALC is

The user types one expression that contains variables (for example `A X^2 + B X + C`), presses CALC, is asked for the value of each variable in turn, and gets the result. CALC can then be pressed again to change the values and recompute without retyping. This is the fx-991ES PLUS function of the same name ("CALC: re-evaluate with variable prompts", register row F068).

What it is **not**: it stores no expression of its own, runs no sequence of steps, has no branch or loop, and keeps nothing after AC or power-off. The expression is the ordinary calculation line. The only data CALC handles beyond a normal calculation is the set of numbers the user typed at the prompts.

## 2. Behaviour

### 2.1 Variables that are prompted

* Candidates: `A B C D E F X Y M`, in that order (Casio order: A to F, then X, Y, M).
* A variable is prompted if it appears in the expression **outside** the body of `integral`, `ddx`, `sum`, `product` and `solve` (those bodies bind `X` internally; `integral(X^2,0,A)` prompts only for `A`). A variable that appears both inside and outside a body is prompted once. Bound-variable rule: decision Q4.
* Each variable is prompted once, however often it appears.
* `Ans`, `PreAns`, constants, matrix and vector names and statistics variables are never prompted.
* If no variable is prompted (for example `2+3`), CALC just evaluates, like `=`.
* The expression must be a plain calculation: an expression with `=` (a Solve equation) gives Syntax ERROR; an empty line does nothing (key rejected, beep).

### 2.2 Prompt screen (192x64, same style as the coefficient grid)

```
 A?                  1/3        title bar: variable name, "n/m" count on the right
 --------------------------
 expression (1 line, scrolled so the prompted variable's first use is visible)

 Value:  2.5_               one-line numeric entry, default text is the current value of the variable
```

* Entry accepts digits, `.`, `(-)`, `x10^x` (exponent) and nothing else. Recall keys (`ALPHA`+variable, `Ans`) paste the *current numeric value* of that variable or Ans. No operators, functions or brackets: this keeps the prompt out of the parser and is the main difference from Casio, which accepts expressions there (decision Q3).
* Maximum 24 characters (the same bound as the statistics and grid editors).
* `=` accepts the entry and moves to the next variable; on the last variable it evaluates.
* `=` with an empty entry keeps the current value of the variable (the default shown).
* `DEL` deletes the last character; the left and right arrow keys move within the entry.
* `AC` cancels: nothing is evaluated, the expression is back in the editor, values already accepted at earlier prompts **stay stored** (see 2.3).
* Up arrow returns to the previous variable's prompt (correction); down arrow does the same forward for variables already visited. No other keys act.

### 2.3 Where the values go (decision Q2, recommended: Casio parity)

Each accepted value is **stored into the variable** (A, B ...), exactly as if the user had pressed `value STO A`. Consequences, all intended:

* the result of CALC is reproducible by ordinary replay (history holds the expression, the variables hold the values);
* variables persist as ordinary memory, as they already do (this is existing behaviour, not CALC state);
* an AC halfway leaves earlier values stored, as on the Casio.

Alternative (session-local values, variables untouched) is listed in section 10; it is safer against surprise overwrites but gives a result that cannot be reproduced by replay.

### 2.4 Result

* The evaluation runs the same evaluator as `=`: same settings (angle, format, exact/decimal, complex, base), same errors (Math ERROR, Syntax ERROR, Can't Solve, Stack ERROR ...).
* The result is shown on the calculation screen with the expression above it, and added to the replay history as an **ordinary history entry** (expression text and result), exactly like `=` (decision Q5).
* After a result, **CALC again** re-opens the prompts from the first variable with the stored values as defaults. Any other key behaves as after a normal result (operator continues from Ans, left/right edits the expression, AC clears).
* After an **error**, CALC again re-opens the prompts (this is the normal way to retry with another value); the left/right arrows show the error cursor as usual.

### 2.5 Availability

* On the calculation screen, in every angle unit, display format, exact/decimal, complex and base-n setting.
* Ignored (rejected, beep) on the menu, statistics, matrix, grid, confirm and logo screens.
* Not available while a multi-valued result is being paged? It is: CALC leaves the paging view.

### 2.6 Key

The hardware has no CALC key. Proposed: **SHIFT + STO** (currently unassigned in both the plain and SHIFT layers; `STO` is not otherwise shifted). Legend printed above STO in the SHIFT colour. Alternatives: SHIFT + DEL (the Casio INS position), or a MENU entry (decision Q1; a menu entry is the slowest and leaves the key free). The physical legend is a hardware-design item and falls under the pending layout legal review.

## 3. State and memory

New fields in `app_t` (RAM, never in the persistence blob):

| Field | Size | Meaning |
|---|---|---|
| `calc_active` | 1 B | 0 idle, 1 prompting, 2 showing a CALC result (so CALC again re-prompts) |
| `calc_mask` | 2 B | bit set of the variables to prompt (9 used) |
| `calc_idx` | 1 B | current variable index |
| `calc_buf[24]`, `calc_len`, `calc_pos` | 26 B | value entry (same shape as `gr_buf`) |

About 30 B of RAM. No new token, no new wire ID (CALC is a key action, not an expression token), so `approved_tokens.txt`, `frozen_token_ids.txt`, `frozen_aux.txt` and the persistence **version stay unchanged (7)**. New screen id `SCR_CALCIN` is appended after `SCR_GRID`.

Stack: the prompt screen runs outside the evaluator (the final evaluation is the same call as `=`), so the worst-case stack must not change. Gate: `tools/check_budget.sh` worst stack must stay at or below 11,512 B (the value after D15) and the CALC build must be measured (risk R15; headroom is already only 6%).

## 4. Session-only guarantees

1. **Persistence:** `persist_save` writes no CALC field. Test: for the same variable values, the blob saved while prompting, while showing a CALC result and while idle is **byte-identical**; and loading any blob leaves `calc_active = 0`.
2. **Power:** auto power-off and OFF during a prompt: after ON the calculator shows the calculation screen with `calc_active = 0` (the expression text is whatever the existing editor restores; that behaviour is unchanged and tested already).
3. **No stored program:** the expression exists only in the editor and (after `=`) the replay history, as for any calculation. CALC adds no second copy.
4. **Guard test:** `test_exclusions` keeps rejecting program, macro, script, table, sheet and other forbidden labels; the key action adds no label. A new check asserts that no menu item or token is named `CALC`.
5. **Source check:** a test fails if `persist.c` or `persist.h` mention any `calc_` field.

## 5. Verification plan

Implementation does not start until section 10 is decided. After approval each item below is built and passes before CALC is called done (feature sequence rule D2).

### 5.1 Key-path tests (`tests/ui/calc.txt`, generated by `tests/tools/make_calc_cases.py`)

The generator writes key sequences with `state:` and screen assertions. Each case: type expression, CALC, values, `=`, expected result and variable state.

* typing and evaluating: 1, 2, 3, 6 and 9 variables; variable order A..F, X, Y, M; a variable used several times prompts once;
* default value shown equals the stored value; `=` on an empty entry keeps it;
* accepted values are stored (check A..M after each case);
* CALC again: prompts restart at the first variable with the new defaults; result changes accordingly;
* AC mid-prompt: expression restored, earlier values stored, no evaluation, `calc_active = 0`;
* up/down correction between prompts;
* left/right after a CALC result edits the expression and ends the CALC state; CALC then re-analyses the edited expression (a new variable gets a prompt);
* each setting combination that changes output: degrees/radians/grads, Norm 1/2, Fix, Sci, Eng, exact/decimal, complex mode;
* integral/derivative/sum/solve bodies with a parameter: `integral(A X^2,0,B)`, `ddx(A X^3,X0)` style, `sum(A^X,1,N)`, `solve(A X=B,1)`; a bound `X` is not prompted; `X` outside a body is prompted.

### 5.2 Independent expected values

Expected results are computed by mpmath at 50 digits from the **substituted** expression (the generator evaluates the formula itself with the chosen values; it never calls the calculator), then rounded to 10 digits for the display and compared. Value sets are chosen as typical classroom formulas with irrational and repeating results:

* quadratic roots `(-B+sqrt(B^2-4AC))/(2A)`, compound interest `A(1+B/100)^C`, projectile range `A^2 sin(2B)/9.81`, Ohm/resistor combinations, trigonometric identities, logarithms, exponentials, powers;
* at least 60 cases, with a third of them using values that give exact fractions and surds.

### 5.3 Boundary and error tests

| Case | Expected |
|---|---|
| expression with no variable | evaluates at once, no prompt |
| expression `X=5` | Syntax ERROR |
| empty line | rejected, beep, no state change |
| entry `1E100` into `A+1` | stores; result per normal overflow rules (Math ERROR where the normal evaluator would give it) |
| entry too long (25th character) | rejected, beep |
| entry `-`, `.`, `E`, `1E` alone | rejected on `=` (stays at the prompt, beep) |
| entry `0` for a divisor | Math ERROR on evaluation; CALC again re-prompts |
| entry with leading zeros, `-0`, `0.000...01` | same as a typed number |
| all nine variables in one expression | nine prompts in the stated order |
| expression at the 256-token and 24-bracket limits with variables | evaluates, or fails exactly as `=` fails |
| recall keys at the prompt (`ALPHA`+B, Ans) | paste the number, never an expression |
| CALC pressed on menu, stats, grid, confirm, logo screens | rejected, beep, no state change |
| CALC during multi-value result paging | leaves paging, prompts |
| power-off at each prompt | back to idle on ON |

### 5.4 Regression coverage

* Full existing suite unchanged and green (`ctest`, 48 tests, CI steps gcc, clang, sanitize, valgrind, wasm, footprint, budget, template).
* Golden files that must **not** change: `tests/exam/approved_tokens.txt`, `frozen_token_ids.txt`, `frozen_aux.txt`, the persistence blob golden of an idle app, `tests/ui/menu_paths.txt`.
* UI fuzz (`test_ui_fuzz`, 40 seeds x 20,000 keys) is extended so the fuzz key set includes the CALC key; sanitizer and valgrind must stay clean.
* Stack and flash budget re-measured; the numbers go into the test-results document.
* The `~>` oracle files are untouched: CALC adds no new mathematics, only a route to the same evaluator, which is why section 5.2 compares against independently substituted values.

### 5.5 Evidence recorded

Coverage register row F068 moves from Missing to Implemented only when 5.1 to 5.4 are all green; the decision log, risk register (R1, R8, R15) and test-results document are updated in the same commit.

## 6. Risks

| Risk | Handling |
|---|---|
| Examination-body concern that CALC holds an expression (R1, R8) | session-only, single expression, no persistence, proved by the byte-identical blob test; build switch (section 8); no approval claim |
| Surprise overwrite of A..M by the prompts | clear prompt screen showing the variable and its current value; Casio parity; alternative in Q2 |
| Stack headroom (6%) | prompt screen outside the evaluator; budget gate must not worsen; measured before approval of the commit |
| Bound-variable rule wrong for unusual nesting | rule is purely syntactic (tokens inside the first argument of the five binding functions); covered by 5.1 including nested and mixed cases |
| Key choice conflicts with the physical layout | key is a decision (Q1); logical key only until the layout is reviewed |

## 7. Out of scope (permanently, Exam model)

Multiple expressions, saving an expression under a name, editing a stored expression, conditional or repeated evaluation, importing or exporting values, and any use of CALC to build a table (TABLE is a separate approved feature).

## 8. Build switch

`TELECO_FEATURE_CALC` (CMake option, default ON in the Exam firmware). With it OFF the key action is absent, `app_t` carries no CALC fields and the key is rejected. CI builds and tests **both** variants, so a regulator-driven removal is a one-line change with a tested result.

## 9. Work plan after approval

1. Parser helper: list of free variables of an expression (outside binding bodies); unit tests first.
2. `SCR_CALCIN` screen, state, key handling; render for the web and PC simulators.
3. Generator and the 5.1 to 5.3 cases; independent values.
4. Persistence-blob and guard tests (section 4).
5. Fuzz extension, full CI, budget re-measurement.
6. Documents and register updates, then report. TABLE does not start before step 6.

## 10. Decisions requested (D18)

| # | Question | Recommendation |
|---|---|---|
| Q1 | Key for CALC | SHIFT + STO |
| Q2 | Prompted values are stored into the variables (Casio parity) or kept session-local | Casio parity |
| Q3 | Prompt accepts a plain number (plus recalled variable or Ans) or a full expression as on the Casio | plain number for the pilot; an expression entry would need a second editor buffer and a second evaluation path; revisit after the stack measurement on hardware |
| Q4 | Variables bound inside integral, ddx, sum, product and solve bodies are not prompted | yes |
| Q5 | CALC results enter the replay history as ordinary entries | yes |
| Q6 | Build switch `TELECO_FEATURE_CALC`, default ON | yes |

Approval of D18 (with any changes to Q1 to Q6) is the gate for implementation.
