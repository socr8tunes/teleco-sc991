"""Pilot question bank. Expected answers are computed independently with mpmath (40 digits) or
stated analytically; they are never produced by the calculator under test."""
import re, pathlib
from keycomp import compile_expr as C, stat_keys, stat_var, menu_pick

ROOT = pathlib.Path(__file__).resolve().parents[2]
CONSTS = [m.group(1) for m in re.finditer(r'\{"[^"]+", "([^"]+)", "', (ROOT / "core/units/constants_table.inc").read_text())]
CONVS = re.findall(r'\{"([^"]+)", "([^"]+)", "[^"]+"\}', (ROOT / "core/units/units.c").read_text().split("CONVS[]")[1].split("};")[0])

def const_keys(sym):
    return " ".join(["SHIFT 7"] + ["DOWN"] * CONSTS.index(sym) + ["EXE"])

def conv_keys(frm, to):
    return "SHIFT 8 " + " ".join(["DOWN"] * CONVS.index((frm, to))) + " EXE"

RAD = "MODE 1 AC"      # switch the angle unit to radians (SETUP > Angle, then leave the menu)

QUESTIONS = []

def Q(level, topic, text, keys, v=None, exact=None, pages=None, tol=1e-9, prefix="", note=""):
    QUESTIONS.append(dict(level=level, topic=topic, text=text, keys=keys, v=v, exact=exact,
                          pages=pages, tol=tol, prefix=prefix, note=note))

S, H, U, T, E = "Secondary school maths", "SHS science", "University maths", "Statistics", "Engineering"

# ---------------------------------------------------------------- Secondary school mathematics
Q(S, "Arithmetic", "Evaluate 2 + 3 x 4^2", C("2+3*4^2"), v="50", exact="50")
Q(S, "Arithmetic", "Evaluate -3^2 (minus applies after the power)", C("-3^2"), v="-9", exact="-9")
Q(S, "Arithmetic", "Evaluate (-3)^2", C("(-3)^2"), v="9", exact="9")
Q(S, "Arithmetic", "Evaluate 1/2pi as 1 divided by 2pi", C("1/2pi"), v="1/(2*pi)")
Q(S, "Percentages", "Find 15% of 240", C("15%*240"), v="36", exact="36")
Q(S, "Percentages", "A price rises from 120 to 150. Percentage increase?", C("(150-120)/120*100"), v="25", exact="25")
Q(S, "Percentages", "Compound interest: 1000 at 5% for 3 years. Final amount?", C("1000*1.05^{3}"), v="1000*mpf('1.05')**3", exact="1157.625")
Q(S, "Fractions", "3/4 + 5/6 as a mixed number", C("frac{3}{4}+frac{5}{6}"), v="mpf(3)/4+mpf(5)/6", exact="1 7/12")
Q(S, "Fractions", "(2/3) divided by (4/9)", C("frac{2}{3}/frac{4}{9}"), v="mpf(3)/2", exact="1 1/2")
Q(S, "Fractions", "1/3 + 1/4 + 1/5", C("frac{1}{3}+frac{1}{4}+frac{1}{5}"), v="mpf(47)/60", exact="47/60")
Q(S, "Fractions", "Simplify 84/126", C("frac{84}{126}"), v="mpf(2)/3", exact="2/3")
Q(S, "Indices", "2^10", C("2^{10}"), v="1024", exact="1024")
Q(S, "Indices", "8^(2/3)", C("8^{frac{2}{3}}"), v="4", exact="4")
Q(S, "Indices", "5^(-2) as a fraction", C("5^{-2}"), v="mpf(1)/25", exact="1/25")
Q(S, "Indices", "Standard form: (6.02 x 10^23) x 2", C("6.02E23*2"), v="mpf('1.204e24')")
Q(S, "Indices", "(3.2 x 10^-4) / (8 x 10^-7)", C("3.2E-4/(8E-7)"), v="400", exact="400")
Q(S, "Surds", "Simplify sqrt(50)", C("sqrt{50}"), v="sqrt(50)", exact="5sqrt(2)")
Q(S, "Surds", "sqrt(12) + sqrt(27)", C("sqrt{12}+sqrt{27}"), v="sqrt(12)+sqrt(27)", exact="5sqrt(3)")
Q(S, "Surds", "Cube root of 27", C("root{3}{27}"), v="3", exact="3")
Q(S, "Surds", "Fourth root of 81", C("root{4}{81}"), v="3", exact="3")
Q(S, "Logarithms", "log 1000", C("log(1000)"), v="3", exact="3")
Q(S, "Logarithms", "ln(e^5)", C("ln(e^{5})"), v="5", exact="5")
Q(S, "Logarithms", "log base 2 of 64 (use log(64)/log(2))", C("log(64)/log(2)"), v="6", exact="6")
Q(S, "Logarithms", "Solve 3^x = 50 for x", C("log(50)/log(3)"), v="log(50)/log(3)")
Q(S, "Trigonometry", "sin 30 degrees", C("sin(30)"), v="mpf(1)/2", exact="1/2")
Q(S, "Trigonometry", "cos 60 degrees", C("cos(60)"), v="mpf(1)/2", exact="1/2")
Q(S, "Trigonometry", "tan 45 degrees", C("tan(45)"), v="1", exact="1")
Q(S, "Trigonometry", "sin 65 degrees", C("sin(65)"), v="sin(radians(65))")
Q(S, "Trigonometry", "Angle whose sine is 0.5 (degrees)", C("asin(0.5)"), v="30", exact="30")
Q(S, "Trigonometry", "Angle whose tangent is 0.75 (degrees)", C("atan(0.75)"), v="degrees(atan(mpf('0.75')))")
Q(S, "Trigonometry", "Hypotenuse of a right triangle with sides 5 and 12", C("sqrt{5^2+12^2}"), v="13", exact="13")
Q(S, "Trigonometry", "Angle B in a triangle with a=7, b=9, A=40 degrees (sine rule), degrees",
  C("asin(9*sin(40)/7)"), v="degrees(asin(9*sin(radians(40))/7))", note="9 sin 40 / 7 = 0.826... so B is acute (the ambiguous case is not tested)")
Q(S, "Trigonometry", "Third side c with a=8, b=11, C=52 degrees (cosine rule)", C("sqrt{8^2+11^2-2*8*11*cos(52)}"),
  v="sqrt(64+121-2*8*11*cos(radians(52)))")
Q(S, "Algebra", "Evaluate 3x^2 - 2x + 1 for x = 2.5", C("2.5->X") + " EXE AC " + C("3X^2-2X+1"), v="mpf('14.75')", exact="14.75")
Q(S, "Algebra", "Solve x^2 - 5x + 6 = 0, first root", C("quad(1,-5,6)"), v="3", exact="3")
Q(S, "Algebra", "Solve x^2 - 5x + 6 = 0, second root", C("quad(1,-5,6)") + " EXE", v="2", exact="2")
Q(S, "Algebra", "Solve 2x^2 + 3x - 5 = 0, first root", C("quad(2,3,-5)"), v="1", exact="1")
Q(S, "Algebra", "Solve 2x^2 + 3x - 5 = 0, second root", C("quad(2,3,-5)") + " EXE", v="mpf(-5)/2", exact="-2 1/2")
Q(S, "Algebra", "Solve x^2 + 2x + 5 = 0 (complex roots allowed), first root", "MODE 4 AC " + C("quad(1,2,5)"), v="mpc(-1,2)", exact="-1+2i")
Q(S, "Algebra", "Simultaneous equations 2x+3y=12, x-y=1: x", C("simul2(2,3,12,1,-1,1)"), v="3", exact="3")
Q(S, "Algebra", "Simultaneous equations 2x+3y=12, x-y=1: y", C("simul2(2,3,12,1,-1,1)") + " EXE", v="2", exact="2")
Q(S, "Algebra", "Solve x^3 - 6x^2 + 11x - 6 = 0, first root shown (others 2 and 3)", C("cubic(1,-6,11,-6)"), v="1", exact="1", note="cubic solver lists roots one at a time; first root is 1 (press the down/right key for 2 and 3)")
Q(S, "Algebra", "Solve 3x + 7 = 25", C("solve(3X+7=25,1)"), v="6", exact="6")
Q(S, "Sequences", "10th term of an arithmetic sequence a=3, d=4", C("3+(10-1)*4"), v="39", exact="39")
Q(S, "Sequences", "Sum of the first 20 terms of a=2, d=3", C("frac{20}{2}*(2*2+19*3)"), v="610", exact="610")
Q(S, "Sequences", "Sum of a geometric series a=2, r=3, n=8", C("2*frac{3^{8}-1}{3-1}"), v="6560", exact="6560")
Q(S, "Probability", "Number of ways to choose 3 from 10", C("10 nCr 3"), v="120", exact="120")
Q(S, "Probability", "Number of arrangements of 4 from 9", C("9 nPr 4"), v="3024", exact="3024")
Q(S, "Probability", "7!", C("7!"), v="5040", exact="5040")
Q(S, "Mensuration", "Area of a circle of radius 7 (exact)", C("pi*7^2"), v="49*pi", exact="49pi")
Q(S, "Mensuration", "Volume of a sphere of radius 3.5", C("frac{4}{3}*pi*3.5^3"), v="4*pi*mpf('3.5')**3/3")
Q(S, "Mensuration", "Arc length: radius 12, angle 70 degrees", C("frac{70}{360}*2*pi*12"), v="70*2*pi*12/360")
Q(S, "Mensuration", "Convert 72 km/h to m/s", C("72") + " " + conv_keys("km/h", "m/s"), v="20", exact="20")
Q(S, "Mensuration", "Convert 5 miles to km", C("5") + " " + conv_keys("mile", "km"), v="mpf('8.04672')")
Q(S, "Mensuration", "Convert 98.6 degF to degC", C("98.6") + " " + conv_keys("degF", "degC"), v="(mpf('98.6')-32)*5/9")

# ---------------------------------------------------------------- SHS science
Q(H, "Physics", "Range of a projectile: v=20 m/s, angle 35 deg, g=9.81 m/s2", C("frac{20^2*sin(2*35)}{9.81}"), v="400*sin(radians(70))/mpf('9.81')")
Q(H, "Physics", "Ohm's law: current for V=12 V, R=47 ohm", C("frac{12}{47}"), v="mpf(12)/47", exact="12/47")
Q(H, "Physics", "Kinetic energy of a 1200 kg car at 25 m/s", C("0.5*1200*25^2"), v="375000", exact="375000")
Q(H, "Physics", "Parallel resistors 10, 15, 30 ohm", C("(1/10+1/15+1/30)^-1"), v="5", exact="5")
Q(H, "Physics", "Energy of a photon, wavelength 500 nm (h c / lambda)", const_keys("h") + " * " + const_keys("c0") + " / " + C("500E-9"),
  v="mpf('6.62607015e-34')*299792458/mpf('500e-9')")
Q(H, "Physics", "Gravitational force: Earth 5.97E24 kg, Moon 7.35E22 kg, r = 3.84E8 m", const_keys("G") + " * " + C("5.97E24*7.35E22/(3.84E8)^2"),
  v="mpf('6.6743e-11')*mpf('5.97e24')*mpf('7.35e22')/mpf('3.84e8')**2")
Q(H, "Physics", "Radioactive decay: N = 1000 e^(-0.0231 x 30)", C("1000*exp(-0.0231*30)"), v="1000*exp(mpf('-0.0231')*30)")
Q(H, "Physics", "Free fall: time to fall 45 m (g = 9.81)", C("sqrt{frac{2*45}{9.81}}"), v="sqrt(2*45/mpf('9.81'))")
Q(H, "Physics", "Snell's law: refraction angle for 40 deg in air into glass n=1.5", C("asin(sin(40)/1.5)"), v="degrees(asin(sin(radians(40))/mpf('1.5')))")
Q(H, "Physics", "Pendulum period, L = 0.75 m, g = 9.81", C("2*pi*sqrt{frac{0.75}{9.81}}"), v="2*pi*sqrt(mpf('0.75')/mpf('9.81'))")
Q(H, "Physics", "Resultant of forces 30 N and 40 N at 90 degrees", C("sqrt{30^2+40^2}"), v="50", exact="50")
Q(H, "Physics", "Power: 2.5 hp in kW", C("2.5") + " " + conv_keys("hp", "kW"), v="mpf('2.5')*mpf('0.74569987158227022192')")
Q(H, "Chemistry", "Moles in 5.85 g NaCl (M = 58.44 g/mol)", C("frac{5.85}{58.44}"), v="mpf('5.85')/mpf('58.44')")
Q(H, "Chemistry", "pH of a solution with [H+] = 3.2E-5 mol/L", C("-log(3.2E-5)"), v="-log10(mpf('3.2e-5'))")
Q(H, "Chemistry", "[H+] for pH 4.75", C("alog(-4.75)"), v="mpf(10)**mpf('-4.75')")
Q(H, "Chemistry", "Ideal gas: pressure for n=2 mol, T=300 K, V=0.05 m3 (R from constants)", C("2*300/0.05") + " * " + const_keys("R"),
  v="mpf(2)*300/mpf('0.05')*mpf('8.31446261815')")
Q(H, "Chemistry", "Number of molecules in 0.25 mol (Avogadro constant)", C("0.25") + " * " + const_keys("NA"), v="mpf('0.25')*mpf('6.02214076e23')")
Q(H, "Chemistry", "Percentage yield: 7.8 g obtained of 9.6 g theoretical", C("frac{7.8}{9.6}*100"), v="mpf('7.8')/mpf('9.6')*100", exact="81.25")
Q(H, "Chemistry", "Dilution: C2 = C1 V1 / V2 = 2.0 x 25 / 100", C("frac{2.0*25}{100}"), v="mpf('0.5')", exact="0.5")
Q(H, "Chemistry", "Temperature 25 degC in K (add the constant for 0 degC)", C("25+") + " " + const_keys("t0"), v="mpf('298.15')")
Q(H, "Biology", "Population growth: 500 x 1.08^12", C("500*1.08^{12}"), v="500*mpf('1.08')**12")
Q(H, "Biology", "Magnification: image 36 mm, specimen 0.45 mm", C("frac{36}{0.45}"), v="80", exact="80")
Q(H, "Biology", "Doubling time: ln2 / 0.035", C("ln(2)/0.035"), v="log(2)/mpf('0.035')")
Q(H, "Biology", "Simpson diversity index: 1 - sum n(n-1)/N(N-1) for n = 10, 6, 4 (N=20)",
  C("1-frac{10*9+6*5+4*3}{20*19}"), v="1-mpf(90+30+12)/380", exact="62/95")

# ---------------------------------------------------------------- University mathematics
Q(U, "Calculus", "Derivative of sin(x) at x=1 (radians)", RAD + " " + C("ddx(sin(X),1)"), v="cos(1)")
Q(U, "Calculus", "Derivative of x^3 - 2x at x=2", C("ddx(X^3-2X,2)"), v="10", exact="10")
Q(U, "Calculus", "Derivative of ln(x) at x=4", C("ddx(ln(X),4)"), v="mpf(1)/4", exact="0.25")
Q(U, "Calculus", "Derivative of e^(x^2) at x=1", C("ddx(exp(X^2),1)"), v="2*e", tol=1e-8)
Q(U, "Calculus", "Integral of x^2 from 0 to 3", C("integral(X^2,0,3)"), v="9", exact="9")
Q(U, "Calculus", "Integral of sin(x) from 0 to pi (radians)", RAD + " " + C("integral(sin(X),0,pi)"), v="2", exact="2")
Q(U, "Calculus", "Integral of 1/x from 1 to 10", C("integral(1/X,1,10)"), v="log(10)")
Q(U, "Calculus", "Integral of e^(-x^2) from 0 to 2", C("integral(exp(-X^2),0,2)"), v="sqrt(pi)/2*erf(2)", tol=1e-8)
Q(U, "Calculus", "Integral of 1/(1+x^2) from -1 to 1", C("integral(1/(1+X^2),-1,1)"), v="pi/2", tol=1e-8)
Q(U, "Calculus", "Area between y=x and y=x^2 from 0 to 1", C("integral(X-X^2,0,1)"), v="mpf(1)/6", tol=1e-9)
Q(U, "Series", "Sum of 1/n^2 for n = 1 to 50", C("sum(1/X^2,1,50)"), v="sum(mpf(1)/n**2 for n in range(1,51))")
Q(U, "Series", "Sum of 2^n for n = 0 to 10", C("sum(2^{X},0,10)"), v="2047", exact="2047")
Q(U, "Series", "Sum of n^3 for n = 1 to 20", C("sum(X^3,1,20)"), v="44100", exact="44100")
Q(U, "Equations", "Root of cos(x) = x (radians), starting at 1", RAD + " " + C("solve(cos(X)=X,1)"), v="findroot(lambda x: cos(x)-x, 0.7)", tol=1e-8)
Q(U, "Equations", "Root of x^3 - 2x - 5 = 0 near 2", C("solve(X^3-2X-5=0,2)"), v="findroot(lambda x: x**3-2*x-5, 2)", tol=1e-8)
Q(U, "Equations", "Root of e^x = 3x near 0.5", C("solve(exp(X)=3X,0.5)"), v="findroot(lambda x: exp(x)-3*x, 0.5)", tol=1e-8)
Q(U, "Equations", "Solve 3x + 2y + z = 14, x - y + 2z = 5, 2x + y - z = 1: x",
  C("simul3(3,2,1,14,1,-1,2,5,2,1,-1,1)"), v="mpf(3)/5", exact="3/5")
Q(U, "Equations", "... same system: y", C("simul3(3,2,1,14,1,-1,2,5,2,1,-1,1)") + " EXE", v="4", exact="4")
Q(U, "Equations", "... same system: z", C("simul3(3,2,1,14,1,-1,2,5,2,1,-1,1)") + " EXE EXE", v="mpf(21)/5", exact="4 1/5")
Q(U, "Complex numbers", "(3+4i)(1-2i)", "MODE 4 AC " + C("(3+4i)*(1-2i)"), v="mpc(11,-2)", exact="11-2i")
Q(U, "Complex numbers", "Modulus of 3+4i", "MODE 4 AC " + C("abs(3+4i)"), v="5", exact="5")
Q(U, "Complex numbers", "Argument of 1+i in degrees", "MODE 4 AC " + C("arg(1+i)"), v="45", exact="45")
Q(U, "Complex numbers", "(1+2i)^10", "MODE 4 AC " + C("(1+2i)^{10}"), v="mpc(1,2)**10", exact="237-3116i")
Q(U, "Complex numbers", "e^(i pi) (radians)", "MODE 4 AC MODE 1 AC " + C("e^{i*pi}"), v="-1", exact="-1")
Q(U, "Complex numbers", "Conjugate of 5-12i", "MODE 4 AC " + C("conj(5-12i)"), v="mpc(5,12)", exact="5+12i")
Q(U, "Matrices", "Determinant of [[2,1],[5,3]]", menu_pick(6, 10) + " [[MENU 6 6]] 2 [[MENU 6 9]] 1 [[MENU 6 8]] 5 [[MENU 6 9]] 3 [[MENU 6 7]]",
  v="1", exact="1")
Q(U, "Matrices", "Determinant of [[1,2,3],[4,5,6],[7,8,10]]", menu_pick(6, 10) + " [[MENU 6 6]] 1 [[MENU 6 9]] 2 [[MENU 6 9]] 3 [[MENU 6 8]] 4 [[MENU 6 9]] 5 [[MENU 6 9]] 6 [[MENU 6 8]] 7 [[MENU 6 9]] 8 [[MENU 6 9]] 1 0 [[MENU 6 7]]",
  v="-3", exact="-3")
Q(U, "Vectors", "Dot product (1,2,3).(4,-5,6)", menu_pick(7, 7) + " [[MENU 7 6]] 1 SHIFT ) 2 SHIFT ) 3 ) SHIFT ) [[MENU 7 6]] 4 SHIFT ) NEG 5 SHIFT ) 6 )",
  v="12", exact="12")
Q(U, "Vectors", "Magnitude of (3,4,12)", menu_pick(7, 11) + " [[MENU 7 6]] 3 SHIFT ) 4 SHIFT ) 1 2 )", v="13", exact="13")
Q(U, "Hyperbolic", "sinh(1.5)", C("sinh(1.5)"), v="sinh(mpf('1.5'))")
Q(U, "Hyperbolic", "cosh^-1(3)", C("acosh(3)"), v="acosh(3)")
Q(U, "Binomial", "Coefficient: 12 choose 5", C("12 nCr 5"), v="792", exact="792")

# ---------------------------------------------------------------- Statistics
DATA1 = [12, 15, 11, 18, 20, 14, 16, 15]
Q(T, "Descriptive", "Mean of 12 15 11 18 20 14 16 15", stat_keys(1, DATA1) + " " + stat_var(3), v="mpf(sum(DATA1))/8".replace("DATA1", str(DATA1)), exact="15 1/8")
Q(T, "Descriptive", "Sum of the same data", stat_keys(1, DATA1) + " " + stat_var(1), v=str(sum(DATA1)), exact=str(sum(DATA1)))
Q(T, "Descriptive", "Population standard deviation of the same data", stat_keys(1, DATA1) + " " + stat_var(4),
  v="sqrt(sum((mpf(x)-mpf(sum(%s))/8)**2 for x in %s)/8)" % (DATA1, DATA1))
Q(T, "Descriptive", "Sample standard deviation of the same data", stat_keys(1, DATA1) + " " + stat_var(5),
  v="sqrt(sum((mpf(x)-mpf(sum(%s))/8)**2 for x in %s)/7)" % (DATA1, DATA1))
Q(T, "Descriptive", "Median of the same data", stat_keys(1, DATA1) + " " + stat_var(21), v="15", exact="15")
Q(T, "Descriptive", "Number of data values", stat_keys(1, DATA1) + " " + stat_var(0), v="8", exact="8")
Q(T, "Descriptive", "Mean of a frequency table: x = 2,4,6,8 with frequencies 3,5,7,5",
  stat_keys(1, [2, 4, 6, 8], freqs=[3, 5, 7, 5]) + " " + stat_var(3), v="mpf(2*3+4*5+6*7+8*5)/20", exact="5 2/5")
Q(T, "Descriptive", "Minimum of the data in the first question", stat_keys(1, DATA1) + " " + stat_var(16), v="11", exact="11")
Q(T, "Descriptive", "Maximum of the data in the first question", stat_keys(1, DATA1) + " " + stat_var(17), v="20", exact="20")
XS, YS = [1, 2, 3, 4, 5, 6], [2.1, 3.9, 6.2, 7.8, 10.1, 12.2]
Q(T, "Regression", "Linear regression y = a + bx: slope b", stat_keys(2, XS, YS) + " " + stat_var(13),
  v="(6*sum(mpf(x)*mpf(str(y)) for x,y in zip(%s,%s))-sum(%s)*sum(mpf(str(y)) for y in %s))/(6*sum(x*x for x in %s)-sum(%s)**2)" % (XS, YS, XS, YS, XS, XS))
Q(T, "Regression", "Linear regression: intercept a", stat_keys(2, XS, YS) + " " + stat_var(12),
  v="(sum(mpf(str(y)) for y in %s)-((6*sum(mpf(x)*mpf(str(y)) for x,y in zip(%s,%s))-sum(%s)*sum(mpf(str(y)) for y in %s))/(6*sum(x*x for x in %s)-sum(%s)**2))*sum(%s))/6" % (YS, XS, YS, XS, YS, XS, XS, XS))
Q(T, "Regression", "Correlation coefficient r", stat_keys(2, XS, YS) + " " + stat_var(15),
  v="corr(%s,%s)" % (XS, YS), tol=1e-8)
Q(T, "Regression", "Estimate y at x = 7 using the regression line", stat_keys(2, XS, YS) + " AC " + menu_pick(8, 5) + " " + C("7"),
  v="yhat(7)", tol=1e-8)
Q(T, "Distribution", "P(Z < 1.5) standard normal", C("normp(1.5)"), v="mpf(1)/2*erfc(-mpf('1.5')/sqrt(2))")
Q(T, "Distribution", "R(1.96) upper tail", C("normr(1.96)"), v="mpf(1)/2*erfc(mpf('1.96')/sqrt(2))", tol=1e-8)
Q(T, "Distribution", "Q(1.96) = area from 0 to 1.96", C("normq(1.96)"), v="mpf(1)/2*erf(mpf('1.96')/sqrt(2))", tol=1e-8)
Q(T, "Distribution", "P(-1 < Z < 1) using R(t) and P(t)", C("normp(1)-normp(-1)"), v="erf(1/sqrt(2))", tol=1e-8)
Q(T, "Distribution", "Standardise: z = (68 - 60) / 5", C("frac{68-60}{5}"), v="mpf(8)/5", exact="1 3/5")
Q(T, "Distribution", "Binomial P(X=3), n=10, p=0.3", C("10 nCr 3*0.3^3*0.7^7"), v="120*mpf('0.3')**3*mpf('0.7')**7")
Q(T, "Distribution", "Poisson P(X=2), lambda=3.5", C("frac{3.5^2*exp(-3.5)}{2!}"), v="mpf('3.5')**2*exp(-mpf('3.5'))/2")
Q(T, "Probability", "Probability of at least one six in 4 rolls", C("1-frac{5}{6}^4"), v="1-(mpf(5)/6)**4", exact="671/1296")

# ---------------------------------------------------------------- Engineering
Q(E, "AC circuits", "Impedance magnitude: R=30, X=40 ohm", "MODE 4 AC " + C("abs(30+40i)"), v="50", exact="50")
Q(E, "AC circuits", "Impedance of R=30 ohm in series with a 40 ohm inductive reactance, phase in degrees", "MODE 4 AC " + C("arg(30+40i)"), v="degrees(atan2(40,30))")
Q(E, "AC circuits", "Series RLC: Z = 20 + j(50-30), |Z|", "MODE 4 AC " + C("abs(20+(50-30)i)"), v="sqrt(20**2+20**2)")
Q(E, "AC circuits", "Resonant frequency: L=10 mH, C=100 nF", C("frac{1}{2*pi*sqrt{0.01*100E-9}}"), v="1/(2*pi*sqrt(mpf('0.01')*mpf('100e-9')))")
Q(E, "AC circuits", "RMS of a 325 V peak sine", C("frac{325}{sqrt{2}}"), v="325/sqrt(2)")
Q(E, "AC circuits", "Power factor: cos(36.87 degrees)", C("cos(36.87)"), v="cos(radians(mpf('36.87')))")
Q(E, "Mechanics", "Stress: 15 kN over 20 mm diameter bar (MPa)", C("frac{15000}{pi*0.01^2}/1E6"), v="mpf(15000)/(pi*mpf('0.01')**2)/mpf(10)**6")
Q(E, "Mechanics", "Moment of inertia of a rectangle 60 x 120 mm: b h^3 / 12 (mm^4)", C("frac{60*120^3}{12}"), v="8640000", exact="8640000")
Q(E, "Mechanics", "Beam deflection: 5wL^4/(384EI), w=10, L=4, E=200E9, I=8E-6", C("frac{5*10*4^4}{384*200E9*8E-6}"), v="mpf(5*10*4**4)/(384*mpf('200e9')*mpf('8e-6'))")
Q(E, "Mechanics", "Convert 150 psi to kPa", C("150") + " " + conv_keys("lbf/in2", "kPa"), v="mpf(150)*mpf('6.8947572931683613')")
Q(E, "Mechanics", "Convert 30 mmHg to Pa", C("30") + " " + conv_keys("mmHg", "Pa"), v="mpf(30)*mpf('133.322387415')")
Q(E, "Polar/rectangular", "Rectangular (3,4) to polar: r", C("pol(3,4)"), v="5", exact="5")
Q(E, "Polar/rectangular", "Rectangular (3,4) to polar: theta", C("pol(3,4)") + " EXE", v="degrees(atan2(4,3))")
Q(E, "Polar/rectangular", "Polar (10, 30 deg) to rectangular: x", C("rec(10,30)"), v="10*cos(radians(30))")
Q(E, "Polar/rectangular", "Polar (10, 30 deg) to rectangular: y", C("rec(10,30)") + " EXE", v="5", exact="5")
Q(E, "Digital", "Hex FF + 1 (Base-N hex)", "MODE 6 5 ALPHA TAN ALPHA TAN + 1", v=None, exact="100")
Q(E, "Digital", "Binary 1011 + 110 (Base-N bin)", "MODE 6 2 1 0 1 1 + 1 1 0", v=None, exact="10001")
Q(E, "Digital", "Octal 777 + 1 (Base-N oct)", "MODE 6 3 7 7 7 + 1", v=None, exact="1000")
Q(E, "Digital", "Binary 1100 AND 1010 (Base-N bin)", "MODE 6 2 1 1 0 0 " + menu_pick(4, 4) + " 1 0 1 0", v=None, exact="1000")
Q(E, "Digital", "Binary 1100 XOR 1010 (Base-N bin)", "MODE 6 2 1 1 0 0 " + menu_pick(4, 6) + " 1 0 1 0", v=None, exact="110")
Q(E, "Thermal", "Heat: m c dT = 2.5 kg x 4186 J/(kg K) x 35 K", C("2.5*4186*35"), v="366275", exact="366275")
Q(E, "Thermal", "Efficiency: 850 / 1200 as a percentage", C("frac{850}{1200}*100"), v="mpf(850)/12", exact="70 5/6")
Q(E, "Thermal", "Stefan-Boltzmann: sigma T^4 for T = 500 K", const_keys("sigma") + " * " + C("500^4"), v="mpf('5.67037441918e-08')*500**4")
Q(E, "Calculus", "Work: integral of 3x^2 + 2x from 1 to 4", C("integral(3X^2+2X,1,4)"), v="78", exact="78")
Q(E, "Calculus", "Average value of sin(x) over 0..pi (radians)", RAD + " " + C("frac{1}{pi}*integral(sin(X),0,pi)"), v="2/pi", tol=1e-8)

# ---------------------------------------------------------------- Number theory and discrete distributions (added with GCD/LCM/PFact/Binom/Poisson/InvN)
Q(S, "Number theory", "Highest common factor of 84 and 126", C("gcd(84,126)"), v="42", exact="42")
Q(S, "Number theory", "Lowest common multiple of 12, 18 (two numbers)", C("lcm(12,18)"), v="36", exact="36")
Q(S, "Number theory", "Two bells ring every 18 and 24 minutes. After how many minutes do they ring together again?", C("lcm(18,24)"), v="72", exact="72")
Q(S, "Number theory", "Prime factors of 360", C("pfact(360)"), exact="2^3x3^2x5")
Q(S, "Number theory", "Prime factors of 1001", C("pfact(1001)"), exact="7x11x13")
Q(S, "Number theory", "Remainder when 2025 is divided by 7", C("rem(2025,7)"), v="2", exact="2")
Q(S, "Number theory", "Whole-number quotient of 2025 divided by 7", C("quot(2025,7)"), v="289", exact="289")
Q(T, "Distribution", "X ~ Binomial(10, 0.5): P(X = 3)", C("binompd(3,10,0.5)"), v="binpd(3,10,'0.5')", tol=1e-9)
Q(T, "Distribution", "X ~ Binomial(12, 0.35): P(X <= 7)", C("binomcd(7,12,0.35)"), v="bincd(7,12,'0.35')", tol=1e-9)
Q(T, "Distribution", "A fair coin is tossed 100 times: P(at most 40 heads)", C("binomcd(40,100,0.5)"), v="bincd(40,100,'0.5')", tol=1e-9)
Q(T, "Distribution", "X ~ Poisson(2.5): P(X = 4)", C("poispd(4,2.5)"), v="poipd(4,'2.5')", tol=1e-9)
Q(T, "Distribution", "Calls arrive at 3 per minute (Poisson): P(at most 2 calls in a minute)", C("poiscd(2,3)"), v="poicd(2,3)", tol=1e-9)
Q(T, "Distribution", "z-value with 97.5% of the standard normal to its left", C("invn(0.975)"), v="invn('0.975')", tol=1e-9)
Q(T, "Distribution", "IQ ~ N(100, 15^2): the score exceeded by only 2.5% of people", C("invn(0.975,15,100)"), v="100+15*invn('0.975')", tol=1e-9)
