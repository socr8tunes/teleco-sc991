// Key table of the Teleco SC991: [script name, primary label, shift label, alpha label].
// The order must match enum key_id in core/ui/keys.h (checked by tests/web/test_wasm.mjs).
export const KEYS = [
  ["SHIFT", "SHIFT", "", ""], ["ALPHA", "ALPHA", "", ""], ["LEFT", "◀", "Home", ""], ["UP", "▲", "Contrast+", ""],
  ["DOWN", "▼", "Contrast−", ""], ["RIGHT", "▶", "End", ""],
  ["MENU", "MENU", "STAT", ""], ["MODE", "SETUP", "RESET", ""], ["FRAC", "a/b", "a b/c", ""],
  ["SQRT", "√", "ⁿ√", ""], ["SQ", "x²", "x³", ""], ["POW", "xⁿ", "", ""],
  ["LOG", "log", "10ˣ", ""], ["LN", "ln", "eˣ", ""], ["NEG", "(−)", "", "A"],
  ["DMS", "°′″", "→DMS", "B"], ["HYP", "hyp", "", "C"], ["SIN", "sin", "sin⁻¹", "D"],
  ["COS", "cos", "cos⁻¹", "E"], ["TAN", "tan", "tan⁻¹", "F"], ["STO", "STO", "", ""],
  ["SD", "S⇔D", "", ""], ["ENG", "ENG", "", ""], ["M+", "M+", "M−", "M"],
  ["(", "(", "%", "Y"], [")", ")", ",", "X"], ["XINV", "x⁻¹", "", ""], ["FACT", "x!", "", ""], ["EE", "×10ˣ", "", ""],
  ["7", "7", "CONST", ""], ["8", "8", "CONV", ""], ["9", "9", "", ""], ["DEL", "DEL", "", ""], ["AC", "AC", "OFF", ""],
  ["4", "4", "", ""], ["5", "5", "", ""], ["6", "6", "", ""], ["*", "×", "nPr", ""], ["/", "÷", "nCr", ""],
  ["1", "1", "", ""], ["2", "2", "", ""], ["3", "3", "", ""], ["+", "+", "Pol(", ""], ["-", "−", "Rec(", ""],
  ["0", "0", "RanInt", ""], [".", ".", "Ran#", ""], ["PI", "π", "e", ""], ["ANS", "Ans", "PreAns", ""], ["EXE", "=", "=", ":"],
];
// Layout: rows of 6, then 5-wide rows; the lower pad reads like a classic calculator.
export const ROWS = [6, 6, 6, 6, 5, 5, 5, 5, 5];
