# Teleco SC991 - quick user guide (pilot)

Switch on with **ON/AC**. A short TELECO logo shows, then the calculation screen. The calculator switches itself off after 10 minutes without a key press; your work and settings are kept.

## Basics
- Type expressions the way you write them. **FRAC** gives a fraction box, **SQRT** a square root, **SHIFT SQRT** an n-th root, **POW** a power; use **RIGHT** to leave the box you are typing in.
- **EXE** calculates. **S<>D** switches between exact (fraction, surd, pi) and decimal answers.
- **Ans** is the last answer. Press an operator right after a result to continue from it.
- Arrow keys move the cursor; **UP/DOWN** recalls earlier calculations. **DEL** deletes, **AC** clears.
- **SHIFT** (yellow labels) and **ALPHA** (variables A B C D E F X Y M) are one-shot: press, then the key.
- **STO** then a variable key stores a value. **M+ / M-** add to / subtract from memory M.

## Menus
- **MODE**: angle unit (Deg/Rad/Grad), number format (Fix n decimals, Sci n significant digits, Norm), complex mode, base-n, statistics, tables.
- **MENU**: complex, matrices, vectors, statistics, calculus (derivative, integral, sum), equation solvers (quadratic, cubic, 2 and 3 unknowns), logic, constants (**SHIFT 7**) and unit conversions (**SHIFT 8**). Pick an item with arrows + EXE, or press its number.
- Statistics: choose the type, enter data in the editor (X, Y, FREQ columns), then read results with MENU > Statistics variables.

## Number theory and distributions
- **MENU > Number**: GCD(a,b), LCM(a,b), Quot(a,b) and Rem(a,b) (whole-number quotient and remainder), PFact(n) (prime factors, e.g. 360 gives 2^3x3^2x5).
- **MENU > Distribution**: normal P/Q/R, InvN (inverse normal), BinomPD/BinomCD (probability of exactly / at most x successes), PoisPD/PoisCD.

## Errors
- **Syntax ERROR**: the expression is incomplete or malformed; the cursor is placed where the problem is.
- **Math ERROR**: the operation is undefined (divide by zero, log of 0, ...).
- **Stack ERROR**: expression too long or too deep. Limits: 24 nested bracket or function levels, 80 number literals, 256 keystrokes (tokens) per line. **Overflow**: result too large.
Press **AC** to clear an error, or an arrow key to go back and edit.

## Tip for testers
The full key reference is in `docs/keymap.md`. Use the test pack (`test-pack.md`) to practise; each question lists the keys used.
