# Teleco SC991 - pilot test pack

Questions with independently verified answers. Testers: do each question using only the keys of the calculator,
compare the displayed result with **Expected**, and record the outcome on the feedback form.
Expected values were computed with mpmath (40 digits) or analytically - never with the calculator. The calculator
shows 10 significant digits, so compare to 10 digits. `Exact` is what the natural display shows first; press S<>D for the decimal.
The **Keys** column is one way to enter the question (the answer must not depend on how it is entered). Angle unit is degrees unless the question says radians.

## Secondary school maths

| ID | Topic | Question | Expected | Exact display | Keys |
|---|---|---|---|---|---|
| Q001 | Arithmetic | Evaluate 2 + 3 x 4^2 | 50 | 50 | `2 + 3 * 4 SQ EXE` |
| Q002 | Arithmetic | Evaluate -3^2 (minus applies after the power) | -9 | -9 | `NEG 3 SQ EXE` |
| Q003 | Arithmetic | Evaluate (-3)^2 | 9 | 9 | `( NEG 3 ) SQ EXE` |
| Q004 | Arithmetic | Evaluate 1/2pi as 1 divided by 2pi | 0.1591549431 | | `1 / 2 PI EXE` |
| Q005 | Percentages | Find 15% of 240 | 36 | 36 | `1 5 SHIFT ( * 2 4 0 EXE` |
| Q006 | Percentages | A price rises from 120 to 150. Percentage increase? | 25 | 25 | `( 1 5 0 - 1 2 0 ) / 1 2 0 * 1 0 0 EXE` |
| Q007 | Percentages | Compound interest: 1000 at 5% for 3 years. Final amount? | 1157.625 | 1157.625 | `1 0 0 0 * 1 . 0 5 POW 3 RIGHT EXE` |
| Q008 | Fractions | 3/4 + 5/6 as a mixed number | 1.583333333 | 1 7/12 | `FRAC 3 DOWN 4 RIGHT + FRAC 5 DOWN 6 RIGHT EXE` |
| Q009 | Fractions | (2/3) divided by (4/9) | 1.5 | 1 1/2 | `FRAC 2 DOWN 3 RIGHT / FRAC 4 DOWN 9 RIGHT EXE` |
| Q010 | Fractions | 1/3 + 1/4 + 1/5 | 0.7833333333 | 47/60 | `FRAC 1 DOWN 3 RIGHT + FRAC 1 DOWN 4 RIGHT + FRAC 1 DOWN 5 RIGHT EXE` |
| Q011 | Fractions | Simplify 84/126 | 0.6666666667 | 2/3 | `FRAC 8 4 DOWN 1 2 6 RIGHT EXE` |
| Q012 | Indices | 2^10 | 1024 | 1024 | `2 POW 1 0 RIGHT EXE` |
| Q013 | Indices | 8^(2/3) | 4 | 4 | `8 POW FRAC 2 DOWN 3 RIGHT RIGHT EXE` |
| Q014 | Indices | 5^(-2) as a fraction | 0.04 | 1/25 | `5 POW NEG 2 RIGHT EXE` |
| Q015 | Indices | Standard form: (6.02 x 10^23) x 2 | 1.204e+24 | | `6 . 0 2 EE 2 3 * 2 EXE` |
| Q016 | Indices | (3.2 x 10^-4) / (8 x 10^-7) | 400 | 400 | `3 . 2 EE NEG 4 / ( 8 EE NEG 7 ) EXE` |
| Q017 | Surds | Simplify sqrt(50) | 7.071067812 | 5sqrt(2) | `SQRT 5 0 RIGHT EXE` |
| Q018 | Surds | sqrt(12) + sqrt(27) | 8.660254038 | 5sqrt(3) | `SQRT 1 2 RIGHT + SQRT 2 7 RIGHT EXE` |
| Q019 | Surds | Cube root of 27 | 3 | 3 | `SHIFT SQRT 3 RIGHT 2 7 RIGHT EXE` |
| Q020 | Surds | Fourth root of 81 | 3 | 3 | `SHIFT SQRT 4 RIGHT 8 1 RIGHT EXE` |
| Q021 | Logarithms | log 1000 | 3 | 3 | `LOG 1 0 0 0 ) EXE` |
| Q022 | Logarithms | ln(e^5) | 5 | 5 | `LN SHIFT PI POW 5 RIGHT ) EXE` |
| Q023 | Logarithms | log base 2 of 64 (use log(64)/log(2)) | 6 | 6 | `LOG 6 4 ) / LOG 2 ) EXE` |
| Q024 | Logarithms | Solve 3^x = 50 for x | 3.560876795 | | `LOG 5 0 ) / LOG 3 ) EXE` |
| Q025 | Trigonometry | sin 30 degrees | 0.5 | 1/2 | `SIN 3 0 ) EXE` |
| Q026 | Trigonometry | cos 60 degrees | 0.5 | 1/2 | `COS 6 0 ) EXE` |
| Q027 | Trigonometry | tan 45 degrees | 1 | 1 | `TAN 4 5 ) EXE` |
| Q028 | Trigonometry | sin 65 degrees | 0.906307787 | | `SIN 6 5 ) EXE` |
| Q029 | Trigonometry | Angle whose sine is 0.5 (degrees) | 30 | 30 | `SHIFT SIN 0 . 5 ) EXE` |
| Q030 | Trigonometry | Angle whose tangent is 0.75 (degrees) | 36.86989765 | | `SHIFT TAN 0 . 7 5 ) EXE` |
| Q031 | Trigonometry | Hypotenuse of a right triangle with sides 5 and 12 | 13 | 13 | `SQRT 5 SQ + 1 2 SQ RIGHT EXE` |
| Q032 | Trigonometry | Angle B in a triangle with a=7, b=9, A=40 degrees (sine rule), degrees | 55.73488131 | | `SHIFT SIN 9 * SIN 4 0 ) / 7 ) EXE` |
| Q033 | Trigonometry | Third side c with a=8, b=11, C=52 degrees (cosine rule) | 8.754631936 | | `SQRT 8 SQ + 1 1 SQ - 2 * 8 * 1 1 * COS 5 2 ) RIGHT EXE` |
| Q034 | Algebra | Evaluate 3x^2 - 2x + 1 for x = 2.5 | 14.75 | 14.75 | `2 . 5 STO ALPHA ) EXE AC 3 ALPHA ) SQ - 2 ALPHA ) + 1 EXE` |
| Q035 | Algebra | Solve x^2 - 5x + 6 = 0, first root | 3 | 3 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 1 1 SHIFT ) NEG 5 SHIFT ) 6 ) EXE` |
| Q036 | Algebra | Solve x^2 - 5x + 6 = 0, second root | 2 | 2 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 1 1 SHIFT ) NEG 5 SHIFT ) 6 ) EXE EXE` |
| Q037 | Algebra | Solve 2x^2 + 3x - 5 = 0, first root | 1 | 1 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 1 2 SHIFT ) 3 SHIFT ) NEG 5 ) EXE` |
| Q038 | Algebra | Solve 2x^2 + 3x - 5 = 0, second root | -2.5 | -2 1/2 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 1 2 SHIFT ) 3 SHIFT ) NEG 5 ) EXE EXE` |
| Q039 | Algebra | Solve x^2 + 2x + 5 = 0 (complex roots allowed), first root | -1.0+2.0i | -1+2i | `MODE 4 AC MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 1 1 SHIFT ) 2 SHIFT ) 5 ) EXE` |
| Q040 | Algebra | Simultaneous equations 2x+3y=12, x-y=1: x | 3 | 3 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 3 2 SHIFT ) 3 SHIFT ) 1 2 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 1 ) EXE` |
| Q041 | Algebra | Simultaneous equations 2x+3y=12, x-y=1: y | 2 | 2 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 3 2 SHIFT ) 3 SHIFT ) 1 2 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 1 ) EXE EXE` |
| Q042 | Algebra | Solve x^3 - 6x^2 + 11x - 6 = 0, first root shown (others 2 and 3) | 1 | 1 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 2 1 SHIFT ) NEG 6 SHIFT ) 1 1 SHIFT ) NEG 6 ) EXE` |
| Q043 | Algebra | Solve 3x + 7 = 25 | 6 | 6 | `MENU 2 4 3 ALPHA ) + 7 SHIFT EXE 2 5 SHIFT ) 1 ) EXE` |
| Q044 | Sequences | 10th term of an arithmetic sequence a=3, d=4 | 39 | 39 | `3 + ( 1 0 - 1 ) * 4 EXE` |
| Q045 | Sequences | Sum of the first 20 terms of a=2, d=3 | 610 | 610 | `FRAC 2 0 DOWN 2 RIGHT * ( 2 * 2 + 1 9 * 3 ) EXE` |
| Q046 | Sequences | Sum of a geometric series a=2, r=3, n=8 | 6560 | 6560 | `2 * FRAC 3 POW 8 RIGHT - 1 DOWN 3 - 1 RIGHT EXE` |
| Q047 | Probability | Number of ways to choose 3 from 10 | 120 | 120 | `1 0 SHIFT / 3 EXE` |
| Q048 | Probability | Number of arrangements of 4 from 9 | 3024 | 3024 | `9 SHIFT * 4 EXE` |
| Q049 | Probability | 7! | 5040 | 5040 | `7 FACT EXE` |
| Q050 | Mensuration | Area of a circle of radius 7 (exact) | 153.93804 | 49pi | `PI * 7 SQ EXE` |
| Q051 | Mensuration | Volume of a sphere of radius 3.5 | 179.59438 | | `FRAC 4 DOWN 3 RIGHT * PI * 3 . 5 SHIFT SQ EXE` |
| Q052 | Mensuration | Arc length: radius 12, angle 70 degrees | 14.66076572 | | `FRAC 7 0 DOWN 3 6 0 RIGHT * 2 * PI * 1 2 EXE` |
| Q053 | Mensuration | Convert 72 km/h to m/s | 20 | 20 | `7 2 SHIFT 8 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q054 | Mensuration | Convert 5 miles to km | 8.04672 | | `5 SHIFT 8 DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q055 | Mensuration | Convert 98.6 degF to degC | 37.0 | | `9 8 . 6 SHIFT 8 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |

## SHS science

| ID | Topic | Question | Expected | Exact display | Keys |
|---|---|---|---|---|---|
| Q056 | Physics | Range of a projectile: v=20 m/s, angle 35 deg, g=9.81 m/s2 | 38.31570319 | | `FRAC 2 0 SQ * SIN 2 * 3 5 ) DOWN 9 . 8 1 RIGHT EXE` |
| Q057 | Physics | Ohm's law: current for V=12 V, R=47 ohm | 0.2553191489 | 12/47 | `FRAC 1 2 DOWN 4 7 RIGHT EXE` |
| Q058 | Physics | Kinetic energy of a 1200 kg car at 25 m/s | 375000 | 375000 | `0 . 5 * 1 2 0 0 * 2 5 SQ EXE` |
| Q059 | Physics | Parallel resistors 10, 15, 30 ohm | 5 | 5 | `( 1 / 1 0 + 1 / 1 5 + 1 / 3 0 ) XINV EXE` |
| Q060 | Physics | Energy of a photon, wavelength 500 nm (h c / lambda) | 3.972891714e-19 | | `SHIFT 7 DOWN DOWN DOWN DOWN DOWN EXE * SHIFT 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE / 5 0 0 EE NEG 9 EXE` |
| Q061 | Physics | Gravitational force: Earth 5.97E24 kg, Moon 7.35E22 kg, r = 3.84E8 m | 1.986117532e+20 | | `SHIFT 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE * 5 . 9 7 EE 2 4 * 7 . 3 5 EE 2 2 / ( 3 . 8 4 EE 8 ) SQ EXE` |
| Q062 | Physics | Radioactive decay: N = 1000 e^(-0.0231 x 30) | 500.0735957 | | `1 0 0 0 * SHIFT LN NEG 0 . 0 2 3 1 * 3 0 ) EXE` |
| Q063 | Physics | Free fall: time to fall 45 m (g = 9.81) | 3.028912664 | | `SQRT FRAC 2 * 4 5 DOWN 9 . 8 1 RIGHT RIGHT EXE` |
| Q064 | Physics | Snell's law: refraction angle for 40 deg in air into glass n=1.5 | 25.37399394 | | `SHIFT SIN SIN 4 0 ) / 1 . 5 ) EXE` |
| Q065 | Physics | Pendulum period, L = 0.75 m, g = 9.81 | 1.737304707 | | `2 * PI * SQRT FRAC 0 . 7 5 DOWN 9 . 8 1 RIGHT RIGHT EXE` |
| Q066 | Physics | Resultant of forces 30 N and 40 N at 90 degrees | 50 | 50 | `SQRT 3 0 SQ + 4 0 SQ RIGHT EXE` |
| Q067 | Physics | Power: 2.5 hp in kW | 1.864249679 | | `2 . 5 SHIFT 8 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q068 | Chemistry | Moles in 5.85 g NaCl (M = 58.44 g/mol) | 0.1001026694 | | `FRAC 5 . 8 5 DOWN 5 8 . 4 4 RIGHT EXE` |
| Q069 | Chemistry | pH of a solution with [H+] = 3.2E-5 mol/L | 4.494850022 | | `NEG LOG 3 . 2 EE NEG 5 ) EXE` |
| Q070 | Chemistry | [H+] for pH 4.75 | 1.77827941e-5 | | `SHIFT LOG NEG 4 . 7 5 ) EXE` |
| Q071 | Chemistry | Ideal gas: pressure for n=2 mol, T=300 K, V=0.05 m3 (R from constants) | 99773.55142 | | `2 * 3 0 0 / 0 . 0 5 * SHIFT 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q072 | Chemistry | Number of molecules in 0.25 mol (Avogadro constant) | 1.50553519e+23 | | `0 . 2 5 * SHIFT 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q073 | Chemistry | Percentage yield: 7.8 g obtained of 9.6 g theoretical | 81.25 | 81.25 | `FRAC 7 . 8 DOWN 9 . 6 RIGHT * 1 0 0 EXE` |
| Q074 | Chemistry | Dilution: C2 = C1 V1 / V2 = 2.0 x 25 / 100 | 0.5 | 0.5 | `FRAC 2 . 0 * 2 5 DOWN 1 0 0 RIGHT EXE` |
| Q075 | Chemistry | Temperature 25 degC in K (add the constant for 0 degC) | 298.15 | | `2 5 + SHIFT 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q076 | Biology | Population growth: 500 x 1.08^12 | 1259.085058 | | `5 0 0 * 1 . 0 8 POW 1 2 RIGHT EXE` |
| Q077 | Biology | Magnification: image 36 mm, specimen 0.45 mm | 80 | 80 | `FRAC 3 6 DOWN 0 . 4 5 RIGHT EXE` |
| Q078 | Biology | Doubling time: ln2 / 0.035 | 19.80420516 | | `LN 2 ) / 0 . 0 3 5 EXE` |
| Q079 | Biology | Simpson diversity index: 1 - sum n(n-1)/N(N-1) for n = 10, 6, 4 (N=20) | 0.6526315789 | 62/95 | `1 - FRAC 1 0 * 9 + 6 * 5 + 4 * 3 DOWN 2 0 * 1 9 RIGHT EXE` |

## University maths

| ID | Topic | Question | Expected | Exact display | Keys |
|---|---|---|---|---|---|
| Q080 | Calculus | Derivative of sin(x) at x=1 (radians) | 0.5403023059 | | `MODE 1 AC MENU 2 2 SIN ALPHA ) ) SHIFT ) 1 ) EXE` |
| Q081 | Calculus | Derivative of x^3 - 2x at x=2 | 10 | 10 | `MENU 2 2 ALPHA ) SHIFT SQ - 2 ALPHA ) SHIFT ) 2 ) EXE` |
| Q082 | Calculus | Derivative of ln(x) at x=4 | 0.25 | 0.25 | `MENU 2 2 LN ALPHA ) ) SHIFT ) 4 ) EXE` |
| Q083 | Calculus | Derivative of e^(x^2) at x=1 | 5.436563657 | | `MENU 2 2 SHIFT LN ALPHA ) SQ ) SHIFT ) 1 ) EXE` |
| Q084 | Calculus | Integral of x^2 from 0 to 3 | 9 | 9 | `MENU 2 3 ALPHA ) SQ SHIFT ) 0 SHIFT ) 3 ) EXE` |
| Q085 | Calculus | Integral of sin(x) from 0 to pi (radians) | 2 | 2 | `MODE 1 AC MENU 2 3 SIN ALPHA ) ) SHIFT ) 0 SHIFT ) PI ) EXE` |
| Q086 | Calculus | Integral of 1/x from 1 to 10 | 2.302585093 | | `MENU 2 3 1 / ALPHA ) SHIFT ) 1 SHIFT ) 1 0 ) EXE` |
| Q087 | Calculus | Integral of e^(-x^2) from 0 to 2 | 0.8820813908 | | `MENU 2 3 SHIFT LN NEG ALPHA ) SQ ) SHIFT ) 0 SHIFT ) 2 ) EXE` |
| Q088 | Calculus | Integral of 1/(1+x^2) from -1 to 1 | 1.570796327 | | `MENU 2 3 1 / ( 1 + ALPHA ) SQ ) SHIFT ) NEG 1 SHIFT ) 1 ) EXE` |
| Q089 | Calculus | Area between y=x and y=x^2 from 0 to 1 | 0.1666666667 | | `MENU 2 3 ALPHA ) - ALPHA ) SQ SHIFT ) 0 SHIFT ) 1 ) EXE` |
| Q090 | Series | Sum of 1/n^2 for n = 1 to 50 | 1.625132734 | | `MENU 2 1 1 / ALPHA ) SQ SHIFT ) 1 SHIFT ) 5 0 ) EXE` |
| Q091 | Series | Sum of 2^n for n = 0 to 10 | 2047 | 2047 | `MENU 2 1 2 POW ALPHA ) RIGHT SHIFT ) 0 SHIFT ) 1 0 ) EXE` |
| Q092 | Series | Sum of n^3 for n = 1 to 20 | 44100 | 44100 | `MENU 2 1 ALPHA ) SHIFT SQ SHIFT ) 1 SHIFT ) 2 0 ) EXE` |
| Q093 | Equations | Root of cos(x) = x (radians), starting at 1 | 0.7390851332 | | `MODE 1 AC MENU 2 4 COS ALPHA ) ) SHIFT EXE ALPHA ) SHIFT ) 1 ) EXE` |
| Q094 | Equations | Root of x^3 - 2x - 5 = 0 near 2 | 2.094551482 | | `MENU 2 4 ALPHA ) SHIFT SQ - 2 ALPHA ) - 5 SHIFT EXE 0 SHIFT ) 2 ) EXE` |
| Q095 | Equations | Root of e^x = 3x near 0.5 | 0.6190612867 | | `MENU 2 4 SHIFT LN ALPHA ) ) SHIFT EXE 3 ALPHA ) SHIFT ) 0 . 5 ) EXE` |
| Q096 | Equations | Solve 3x + 2y + z = 14, x - y + 2z = 5, 2x + y - z = 1: x | 0.6 | 3/5 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 4 3 SHIFT ) 2 SHIFT ) 1 SHIFT ) 1 4 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 2 SHIFT ) 5 SHIFT ) 2 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 1 ) EXE` |
| Q097 | Equations | ... same system: y | 4 | 4 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 4 3 SHIFT ) 2 SHIFT ) 1 SHIFT ) 1 4 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 2 SHIFT ) 5 SHIFT ) 2 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 1 ) EXE EXE` |
| Q098 | Equations | ... same system: z | 4.2 | 4 1/5 | `MENU DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE 4 3 SHIFT ) 2 SHIFT ) 1 SHIFT ) 1 4 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 2 SHIFT ) 5 SHIFT ) 2 SHIFT ) 1 SHIFT ) NEG 1 SHIFT ) 1 ) EXE EXE EXE` |
| Q099 | Complex numbers | (3+4i)(1-2i) | 11.0-2.0i | 11-2i | `MODE 4 AC ( 3 + 4 MENU 5 1 ) * ( 1 - 2 MENU 5 1 ) EXE` |
| Q100 | Complex numbers | Modulus of 3+4i | 5 | 5 | `MODE 4 AC MENU 3 1 3 + 4 MENU 5 1 ) EXE` |
| Q101 | Complex numbers | Argument of 1+i in degrees | 45 | 45 | `MODE 4 AC MENU 5 2 1 + MENU 5 1 ) EXE` |
| Q102 | Complex numbers | (1+2i)^10 | 237.0-3116.0i | 237-3116i | `MODE 4 AC ( 1 + 2 MENU 5 1 ) POW 1 0 RIGHT EXE` |
| Q103 | Complex numbers | e^(i pi) (radians) | -1 | -1 | `MODE 4 AC MODE 1 AC SHIFT PI POW MENU 5 1 * PI RIGHT EXE` |
| Q104 | Complex numbers | Conjugate of 5-12i | 5.0+12.0i | 5+12i | `MODE 4 AC MENU 5 3 5 - 1 2 MENU 5 1 ) EXE` |
| Q105 | Matrices | Determinant of [[2,1],[5,3]] | 1 | 1 | `MENU 6 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE MENU 6 6 2 MENU 6 9 1 MENU 6 8 5 MENU 6 9 3 MENU 6 7 EXE` |
| Q106 | Matrices | Determinant of [[1,2,3],[4,5,6],[7,8,10]] | -3 | -3 | `MENU 6 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE MENU 6 6 1 MENU 6 9 2 MENU 6 9 3 MENU 6 8 4 MENU 6 9 5 MENU 6 9 6 MENU 6 8 7 MENU 6 9 8 MENU 6 9 1 0 MENU 6 7 EXE` |
| Q107 | Vectors | Dot product (1,2,3).(4,-5,6) | 12 | 12 | `MENU 7 7 MENU 7 6 1 SHIFT ) 2 SHIFT ) 3 ) SHIFT ) MENU 7 6 4 SHIFT ) NEG 5 SHIFT ) 6 ) EXE` |
| Q108 | Vectors | Magnitude of (3,4,12) | 13 | 13 | `MENU 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE MENU 7 6 3 SHIFT ) 4 SHIFT ) 1 2 ) EXE` |
| Q109 | Hyperbolic | sinh(1.5) | 2.129279455 | | `MENU 1 1 1 . 5 ) EXE` |
| Q110 | Hyperbolic | cosh^-1(3) | 1.762747174 | | `MENU 1 5 3 ) EXE` |
| Q111 | Binomial | Coefficient: 12 choose 5 | 792 | 792 | `1 2 SHIFT / 5 EXE` |

## Statistics

| ID | Topic | Question | Expected | Exact display | Keys |
|---|---|---|---|---|---|
| Q112 | Descriptive | Mean of 12 15 11 18 20 14 16 15 | 15.125 | 15 1/8 | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN DOWN DOWN EXE EXE` |
| Q113 | Descriptive | Sum of the same data | 121 | 121 | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN EXE EXE` |
| Q114 | Descriptive | Population standard deviation of the same data | 2.758509561 | | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN EXE EXE` |
| Q115 | Descriptive | Sample standard deviation of the same data | 2.948970765 | | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q116 | Descriptive | Median of the same data | 15 | 15 | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q117 | Descriptive | Number of data values | 8 | 8 | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3  EXE EXE` |
| Q118 | Descriptive | Mean of a frequency table: x = 2,4,6,8 with frequencies 3,5,7,5 | 5.4 | 5 2/5 | `MODE 7 1 2 EXE 4 EXE 6 EXE 8 EXE UP UP UP UP  RIGHT 3 EXE 5 EXE 7 EXE 5 EXE AC MENU 8 3 DOWN DOWN DOWN EXE EXE` |
| Q119 | Descriptive | Minimum of the data in the first question | 11 | 11 | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q120 | Descriptive | Maximum of the data in the first question | 20 | 20 | `MODE 7 1 12 EXE 15 EXE 11 EXE 18 EXE 20 EXE 14 EXE 16 EXE 15 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q121 | Regression | Linear regression y = a + bx: slope b | 2.02 | | `MODE 7 2 1 EXE 2 EXE 3 EXE 4 EXE 5 EXE 6 EXE UP UP UP UP UP UP  RIGHT 2.1 EXE 3.9 EXE 6.2 EXE 7.8 EXE 10.1 EXE 12.2 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q122 | Regression | Linear regression: intercept a | -0.02 | | `MODE 7 2 1 EXE 2 EXE 3 EXE 4 EXE 5 EXE 6 EXE UP UP UP UP UP UP  RIGHT 2.1 EXE 3.9 EXE 6.2 EXE 7.8 EXE 10.1 EXE 12.2 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q123 | Regression | Correlation coefficient r | 0.9991049325 | | `MODE 7 2 1 EXE 2 EXE 3 EXE 4 EXE 5 EXE 6 EXE UP UP UP UP UP UP  RIGHT 2.1 EXE 3.9 EXE 6.2 EXE 7.8 EXE 10.1 EXE 12.2 EXE AC MENU 8 3 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q124 | Regression | Estimate y at x = 7 using the regression line | 14.12 | | `MODE 7 2 1 EXE 2 EXE 3 EXE 4 EXE 5 EXE 6 EXE UP UP UP UP UP UP  RIGHT 2.1 EXE 3.9 EXE 6.2 EXE 7.8 EXE 10.1 EXE 12.2 EXE AC MENU 8 5 7 EXE` |
| Q125 | Distribution | P(Z < 1.5) standard normal | 0.9331927987 | | `MENU 9 1 1 . 5 ) EXE` |
| Q126 | Distribution | R(1.96) upper tail | 0.02499789515 | | `MENU 9 3 1 . 9 6 ) EXE` |
| Q127 | Distribution | Q(1.96) = area from 0 to 1.96 | 0.4750021049 | | `MENU 9 2 1 . 9 6 ) EXE` |
| Q128 | Distribution | P(-1 < Z < 1) using R(t) and P(t) | 0.6826894921 | | `MENU 9 1 1 ) - MENU 9 1 NEG 1 ) EXE` |
| Q129 | Distribution | Standardise: z = (68 - 60) / 5 | 1.6 | 1 3/5 | `FRAC 6 8 - 6 0 DOWN 5 RIGHT EXE` |
| Q130 | Distribution | Binomial P(X=3), n=10, p=0.3 | 0.266827932 | | `1 0 SHIFT / 3 * 0 . 3 SHIFT SQ * 0 . 7 POW 7 RIGHT EXE` |
| Q131 | Distribution | Poisson P(X=2), lambda=3.5 | 0.1849589735 | | `FRAC 3 . 5 SQ * SHIFT LN NEG 3 . 5 ) DOWN 2 FACT RIGHT EXE` |
| Q132 | Probability | Probability of at least one six in 4 rolls | 0.5177469136 | 671/1296 | `1 - FRAC 5 DOWN 6 RIGHT POW 4 RIGHT EXE` |

## Engineering

| ID | Topic | Question | Expected | Exact display | Keys |
|---|---|---|---|---|---|
| Q133 | AC circuits | Impedance magnitude: R=30, X=40 ohm | 50 | 50 | `MODE 4 AC MENU 3 1 3 0 + 4 0 MENU 5 1 ) EXE` |
| Q134 | AC circuits | Impedance of R=30 ohm in series with a 40 ohm inductive reactance, phase in degrees | 53.13010235 | | `MODE 4 AC MENU 5 2 3 0 + 4 0 MENU 5 1 ) EXE` |
| Q135 | AC circuits | Series RLC: Z = 20 + j(50-30), |Z| | 28.28427125 | | `MODE 4 AC MENU 3 1 2 0 + ( 5 0 - 3 0 ) MENU 5 1 ) EXE` |
| Q136 | AC circuits | Resonant frequency: L=10 mH, C=100 nF | 5032.92121 | | `FRAC 1 DOWN 2 * PI * SQRT 0 . 0 1 * 1 0 0 EE NEG 9 RIGHT RIGHT EXE` |
| Q137 | AC circuits | RMS of a 325 V peak sine | 229.8097039 | | `FRAC 3 2 5 DOWN SQRT 2 RIGHT RIGHT EXE` |
| Q138 | AC circuits | Power factor: cos(36.87 degrees) | 0.7999989281 | | `COS 3 6 . 8 7 ) EXE` |
| Q139 | Mechanics | Stress: 15 kN over 20 mm diameter bar (MPa) | 47.74648293 | | `FRAC 1 5 0 0 0 DOWN PI * 0 . 0 1 SQ RIGHT / 1 EE 6 EXE` |
| Q140 | Mechanics | Moment of inertia of a rectangle 60 x 120 mm: b h^3 / 12 (mm^4) | 8640000 | 8640000 | `FRAC 6 0 * 1 2 0 SHIFT SQ DOWN 1 2 RIGHT EXE` |
| Q141 | Mechanics | Beam deflection: 5wL^4/(384EI), w=10, L=4, E=200E9, I=8E-6 | 2.083333333e-5 | | `FRAC 5 * 1 0 * 4 POW 4 RIGHT DOWN 3 8 4 * 2 0 0 EE 9 * 8 EE NEG 6 RIGHT EXE` |
| Q142 | Mechanics | Convert 150 psi to kPa | 1034.213594 | | `1 5 0 SHIFT 8 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q143 | Mechanics | Convert 30 mmHg to Pa | 3999.671622 | | `3 0 SHIFT 8 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE EXE` |
| Q144 | Polar/rectangular | Rectangular (3,4) to polar: r | 5 | 5 | `SHIFT + 3 SHIFT ) 4 ) EXE` |
| Q145 | Polar/rectangular | Rectangular (3,4) to polar: theta | 53.13010235 | | `SHIFT + 3 SHIFT ) 4 ) EXE EXE` |
| Q146 | Polar/rectangular | Polar (10, 30 deg) to rectangular: x | 8.660254038 | | `SHIFT - 1 0 SHIFT ) 3 0 ) EXE` |
| Q147 | Polar/rectangular | Polar (10, 30 deg) to rectangular: y | 5 | 5 | `SHIFT - 1 0 SHIFT ) 3 0 ) EXE EXE` |
| Q148 | Digital | Hex FF + 1 (Base-N hex) | | 100 | `MODE 6 5 ALPHA TAN ALPHA TAN + 1 EXE` |
| Q149 | Digital | Binary 1011 + 110 (Base-N bin) | | 10001 | `MODE 6 2 1 0 1 1 + 1 1 0 EXE` |
| Q150 | Digital | Octal 777 + 1 (Base-N oct) | | 1000 | `MODE 6 3 7 7 7 + 1 EXE` |
| Q151 | Digital | Binary 1100 AND 1010 (Base-N bin) | | 1000 | `MODE 6 2 1 1 0 0 MENU 4 4 1 0 1 0 EXE` |
| Q152 | Digital | Binary 1100 XOR 1010 (Base-N bin) | | 110 | `MODE 6 2 1 1 0 0 MENU 4 6 1 0 1 0 EXE` |
| Q153 | Thermal | Heat: m c dT = 2.5 kg x 4186 J/(kg K) x 35 K | 366275 | 366275 | `2 . 5 * 4 1 8 6 * 3 5 EXE` |
| Q154 | Thermal | Efficiency: 850 / 1200 as a percentage | 70.83333333 | 70 5/6 | `FRAC 8 5 0 DOWN 1 2 0 0 RIGHT * 1 0 0 EXE` |
| Q155 | Thermal | Stefan-Boltzmann: sigma T^4 for T = 500 K | 3543.984012 | | `SHIFT 7 DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN DOWN EXE * 5 0 0 POW 4 RIGHT EXE` |
| Q156 | Calculus | Work: integral of 3x^2 + 2x from 1 to 4 | 78 | 78 | `MENU 2 3 3 ALPHA ) SQ + 2 ALPHA ) SHIFT ) 1 SHIFT ) 4 ) EXE` |
| Q157 | Calculus | Average value of sin(x) over 0..pi (radians) | 0.6366197724 | | `MODE 1 AC FRAC 1 DOWN PI RIGHT * MENU 2 3 SIN ALPHA ) ) SHIFT ) 0 SHIFT ) PI ) EXE` |
