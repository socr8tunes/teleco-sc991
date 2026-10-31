# Teleco SC991 keymap

Logical keys are defined in `core/ui/keys.h`; the platform maps physical scan codes to them.
The browser simulator (`platform/web`) draws this layout with its legends.

```
 SHIFT   ALPHA    <      ^      v      >
 MENU    SETUP    a/b    sqrt   x^2    x^n
 log     ln       (-)    DMS    hyp    sin
 cos     tan      STO    S<>D   ENG    M+
 (       )        x^-1   x!     x10^x
 7       8        9      DEL    AC
 4       5        6      x      /
 1       2        3      +      -
 0       .        pi     Ans    =
```

| Key | Plain | SHIFT | ALPHA |
|---|---|---|---|
| `<` `>` | cursor left / right (after a result: edit the expression) | Home / End | |
| `^` `v` | history back / forward, move between numerator and denominator, page multi-value results | contrast + / - | |
| MENU | function catalogue (hyperbolic, calculus, number, probability/logic, complex, matrix, vector, statistics, distribution, equations, memory, angle units, constants, unit conversion) | statistics data editor | |
| SETUP | angle, display format (Norm 1/2, Fix, Sci, Eng), fraction style, complex mode and form, base-n, regression type, reset | **RESET** (asks first) | |
| a/b | stacked fraction (a number just typed becomes the numerator) | mixed fraction | |
| sqrt | square root | n-th root | |
| x^2 | square | cube | |
| x^n | superscript (any expression) | | |
| log / ln | log10( / ln( | 10^( / e^( | |
| (-) | negative sign | | A |
| DMS | degree / minute / second mark | switch result to DMS | B |
| hyp | hyperbolic menu | | C |
| sin cos tan | functions | inverse functions | D E F |
| STO | store: then press a variable (ALPHA + key), a Mat / Vct item from MENU | | |
| S<>D | exact (fractions, surds, pi) <-> decimal result | | |
| ENG | engineering notation for this result | | |
| M+ | M = M + result | M = M - result | M |
| ( ) | parentheses | % and , | Y, X |
| x^-1 x! | reciprocal, factorial | | |
| x10^x | exponent entry | | |
| 7 | | constants list | |
| 8 | | unit conversions | |
| AC | clear the line (memory kept) | **OFF** | |
| x / | multiply / divide | nPr / nCr | |
| + - | add / subtract | Pol( / Rec( | |
| 0 . | | RanInt( / Ran# | |
| pi | pi | e | |
| Ans | last answer | PreAns | |
| = | calculate | `=` sign (for Solve) | `:` |

After a result, an operator key starts from `Ans` (for example `x` then `2`); a digit or function key starts a new line.
In **Hex** mode ALPHA + (-), DMS, hyp, sin, cos, tan enter the digits A-F.
Variables A B C D E F X Y M are ALPHA keys; statistics variables, constants and conversions come from MENU.

Memory (variables, Ans, history, statistics data, settings) survives AC, auto power-off and OFF.
Only SHIFT + SETUP (RESET) clears it, after a confirmation.
