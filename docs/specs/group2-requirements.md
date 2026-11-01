# Requirements for the next feature group (for confirmation before implementation)

Status: **proposed, not implemented.** Product constraints that apply to everything below: non-programmable (no stored user programs, macros,
scripts, persistent formulas, spreadsheet), natural display, 10 significant digits shown, strict error handling (Math ERROR / Argument ERROR /
Syntax ERROR / Time Out), every feature reachable and tested through the key interface, independent expected values (mpmath / exact fractions), never the calculator's own output.

## 0. TABLE and CALC - can they be restored without breaking "non-programmable"?
Reference (Casio fx-570/991ES PLUS manual, public): CALC stores an expression that contains variables, prompts for each variable's value and shows the result,
repeatable with new values; TABLE generates a read-only table of x and f(x) from one function, Start/End/Step, at most 30 rows, the function may not contain
Pol, Rec, integral, d/dx or sum. That calculator is non-programmable, so neither feature is programming in itself. What *would* cross the line is anything that
stores logic persistently or lets the user chain statements. Proposed rules that keep both features inside the line:

| Rule | CALC | TABLE |
|---|---|---|
| What is held | the single expression currently on the entry line (a mathematical expression, nothing else) | one function f(X) |
| Not allowed | ":" chaining (already removed), assignments like A=B+C as stored equations, any statement list | a second function g(X), ":" chaining, any control logic |
| Lifetime | session only: discarded on AC, on leaving CALC, on any menu/mode change and on power-off | session only: discarded on leaving TABLE, on power-off |
| Persistence | **never saved** in the retained-state blob and never survives a firmware or power cycle | **never saved** |
| Output | one result per set of variable values (no loops, no conditions) | view-only table, <= 30 rows (error if more), no editing of cells |
| Excluded functions in the expression | none beyond the normal rule that Solve( inside CALC is fine because it is evaluated once | Pol(, Rec(, integral, d/dx, sum, product, Solve(, matrix and vector values |
| Variables | A-F, X, Y, M prompted one by one, current values shown as defaults; assigning the value does overwrite the variable (as on Casio) | only X; other variables act as constants |
| Guard tests | after leaving, the expression text is gone from ctx/persist (checked by a byte scan of the saved blob); no key sequence can store, name, call or loop an expression | same |

Existing replay history (UP/DOWN recall of previously entered lines) already stores typed lines; it stays as is. CALC/TABLE do not add a second store.
**Decision for you:** restore both under these rules (my recommendation), or only TABLE, or neither. Row limit 30 and "single f(x)" are conservative defaults; the fx-991EX allows f(x) and g(x) and 45 rows.

## 1. Product operator  Π( f(X), a, b )
Mathematics: Π_{X=a}^{b} f(X), a and b integers, step 1, X is the loop variable (restored afterwards), like the existing Σ(.
- Empty range (a > b) = 1 (empty product). Σ's empty range is 0, so this mirrors it.
- Range limit: b - a <= 100 000, else Time Out (same as Σ). Non-integer bounds: Argument ERROR. Complex or matrix terms: Argument ERROR.
- Overflow of the running product: Math ERROR. A term that is 0 gives 0 (no early exit, so an error in a later term still reports).
- Underflow flushes to 0 like multiplication elsewhere. Result is exact when every term is exact (integers/fractions), e.g. Π(X,1,10) = 3628800, Π(1/X,1,3) = 1/6.
- Key: MENU > Calculus > Π(, entered as a block function with the same editor/parse rules as Σ. Display glyph: capital pi, distinct from the constant pi.
- Tests: Π(X,1,5)=120, Π(X,1,20) exact vs 20!, Π(X,1,0)=1, Π(2,1,10)=1024, Π(1+1/X,1,9)=10, Π(X,3,3)=3, Π(X,5,1)=1, negative ranges Π(X,-2,2)=0, Π(X^2+1,1,4) = 2*5*10*17, overflow Π(X,1,100)->Math ERROR, bounds limit, non-integer bound, nested inside expressions and with Ans/variables, key-sequence tests, pilot questions.

## 2. Fourth-degree polynomial   Quartic( a, b, c, d, e )  :  a x^4 + b x^3 + c x^2 + d x + e = 0
- Requires a != 0 (else Argument ERROR). Returns **four roots with multiplicity** (repeated roots appear repeatedly), shown x1..x4 on pages like Cubic.
- Order: real roots ascending first, then complex roots as conjugate pairs with positive imaginary part first (same convention as Cubic).
- Complex roots are always returned (as for Quad/Cubic, independent of the CMPLX setting), shown a+bi.
- Accuracy target: each root correct to the 10 displayed digits (relative error <= 1e-9 for roots of well-conditioned polynomials; checked by residual |p(x)| and against mpmath polyroots at 50 digits).
- Method (proposal): exact detection of repeated factors when the coefficients are integers/decimals (gcd of p and p' in exact arithmetic), then closed-form Ferrari/resolvent cubic or Durand-Kerner in extended precision with Newton polishing. A 4-fold root such as (x-1)^4 must show 1, not 1.0000001: this is why the exact step is required.
- Exact output where roots are rational (e.g. x^4 - 5x^2 + 4 -> -2, -1, 1, 2 shown as integers; roots like sqrt(2) shown as sqrt forms if the existing exact layer supports them, otherwise decimals).
- Errors: coefficient non-real -> Argument ERROR; overflow in intermediate -> Math ERROR.
- Tests: factored polynomials with known roots; biquadratics x^4+4 (=±1±i); x^4-1; (x-1)^4, (x-1)^2(x+2)^2, (x-1)^3(x-2); all-complex; large/small coefficient scales (1e6, 1e-6); near-multiple roots (Wilkinson-style (x-1)(x-1.001)...); Vieta checks (sum, pairwise sums, product); random polynomials vs mpmath polyroots (1000 cases); a=0, non-numbers; key-sequence tests; pilot questions (projectile, beam deflection, vibration).

## 3. Four-unknown simultaneous equations   Simul4( 20 values )
Mathematics: solve A x = b for a 4x4 system, values given row by row as (a1,b1,c1,d1,k1, ... , a4,b4,c4,d4,k4), exactly as Simul2/Simul3.
- Unique solution: x, y, z, t shown on four pages (labels x=, y=, z=, t=), exact fractions when the inputs are exact.
- Singular or numerically singular system (zero determinant, determined exactly for exact inputs): Math ERROR (no unique solution). The calculator does not distinguish "no solution" from "infinitely many".
- Accuracy: exact rational arithmetic for exact inputs; for ill-conditioned systems (e.g. Hilbert 4x4) the answer must still be exact when inputs are integers.
- Tests: systems generated from chosen integer/fraction solutions and checked with Python Fractions; singular systems (dependent rows, zero row); permuted rows needing pivoting (zero on the diagonal); Hilbert 4x4; decimals; large values; key tests.
- UI: function form (consistent with Simul3). A 4x5 grid editor like Casio's is a possible later improvement (pending your preference).

## 4. Inequality solver (degree 2 to 4)   operator in { >, <, >=, <= } :  a x^n + ... + e  (op) 0
- Right side is 0 (as on Casio). Degree n = 2, 3 or 4 chosen by the form; leading coefficient != 0.
- Solution set is shown as a union of intervals in increasing order, one interval per page (or on one line when short), endpoints as decimals (10 digits) or exact when the roots are exact.
  Notation (text): `x<-1`, `-1<x<2`, `x<=-1`, `x>=3`, with several intervals shown as separate pages "1/2: x<-1", "2/2: x>2".
- Special results: `All real numbers` (e.g. x^2+1>0), `No solution` (e.g. x^2+1<0), single point `x=0` (x^2<=0), punctured line `x<0` and `x>0` (x^2>0).
- Mathematics: real roots of the polynomial (multiplicity matters: an even-multiplicity root does not change the sign), sign of the polynomial on each interval determined from the leading coefficient and root multiplicities (not by evaluating near the roots), strict operators exclude endpoints, non-strict include them.
- Roots computed with the same polynomial solver as sections 2 and Quad/Cubic, so a repeated root is recognised exactly.
- UI: MENU > Equations > Inequality opens a small form (choose operator, choose degree, then enter the 3-5 coefficients in an editor like the stat editor); EXE shows the solution. Nothing is stored after leaving the form.
- Tests: all 4 operators x degrees 2,3,4; no real roots (both signs of a); one double root; two roots; three roots with a double; four distinct roots; x^4 type with two double roots; roots that are irrational (compare to mpmath roots at 50 digits); strictness at roots; a=0 error; scale tests; key-sequence tests and pilot questions.

## Questions for you before I implement
1. TABLE/CALC: restore under the rules in section 0? (recommended: yes, both)
2. Inequality display: pages per interval (as written) or one scrolling line?
3. Simul4 as a function (consistent, quick) now, grid editor later?
4. Quartic root presentation: decimal always, or exact (integers, fractions, sqrt) when available, like Quad/Cubic?
