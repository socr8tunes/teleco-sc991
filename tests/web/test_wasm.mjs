// Runs the key-driven UI cases against the WebAssembly build (differential test of native vs wasm).
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { KEYS } from "../../platform/web/keys.js";

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, "../..");
const wasmPath = process.argv[2] || path.join(root, "build-web/teleco.wasm");
const casesPath = process.argv[3] || path.join(root, "tests/ui/keys.txt");

const { instance } = await WebAssembly.instantiate(fs.readFileSync(wasmPath), {});
const w = instance.exports;
const mem = () => new Uint8Array(w.memory.buffer);
const cstr = (p) => { const m = mem(); let e = p; while (m[e]) e++; return Buffer.from(m.subarray(p, e)).toString("latin1"); };

let fails = 0, checks = 0;
if (w.teleco_key_count() !== KEYS.length) { console.log(`key table mismatch: wasm ${w.teleco_key_count()} js ${KEYS.length}`); process.exit(1); }
const byName = new Map(KEYS.map((k, i) => [k[0], i]));
byName.set("x", byName.get("*")); byName.set("=", byName.get("EXE"));
const digit = (c) => byName.get(c === "." ? "." : c);

function loadExpr(text) {
  const p = w.teleco_text_buf();
  const bytes = Buffer.from(text, "latin1");
  mem().set(bytes, p); mem()[p + bytes.length] = 0;
  return w.teleco_load_expr();
}

let now = 0;
for (const raw of fs.readFileSync(casesPath, "utf8").split("\n")) {
  const line = raw.trim();
  if (!line || line.startsWith("#")) continue;
  const i = line.indexOf("=>");
  let keys = line.slice(0, i).trim(), exp = line.slice(i + 2).trim();
  if (exp.startsWith("expr:") || exp === "@expr") continue;       // editor text is checked natively
  w.teleco_init(); now = 5000; w.teleco_tick(now); w.teleco_key(byName.get("AC"));
  if (keys.startsWith("{")) {
    const end = keys.indexOf("}");
    if (loadExpr(keys.slice(1, end))) { console.log("cannot load", line); fails++; continue; }
    keys = keys.slice(end + 1).trim();
  }
  checks++;
  for (const word of keys.split(/\s+/).filter(Boolean)) {
    if (word.startsWith("wait:")) { now += +word.slice(5); w.teleco_tick(now); continue; }
    if (/^[0-9.]+$/.test(word) && word.length > 1) { for (const c of word) w.teleco_key(digit(c)); continue; }
    if (!byName.has(word)) { console.log("unknown key", word); fails++; break; }
    w.teleco_key(byName.get(word));
  }
  const got = cstr(w.teleco_result());
  if (got !== exp) { console.log(`FAIL [${keys}] got '${got}' expected '${exp}'`); fails++; }
}

// framebuffer sanity: dimensions and the logo screen is not blank
w.teleco_init();
const fb = w.teleco_render();
const stride = w.teleco_stride(), h = w.teleco_height();
let dark = 0;
for (let k = 0; k < stride * h; k++) dark += mem()[fb + k].toString(2).split("1").length - 1;
if (w.teleco_width() !== 192 || h !== 64 || dark < 200) { console.log(`bad framebuffer (dark=${dark})`); fails++; }
checks++;
console.log(`wasm: ${checks} checks, ${fails} failures`);
process.exit(fails ? 1 : 0);
