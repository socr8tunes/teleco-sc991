#!/usr/bin/env python3
"""Embed teleco.wasm and keys.js into one self-contained simulator page."""
import base64, pathlib, re, sys
root = pathlib.Path(__file__).resolve().parent
out = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else root.parent.parent / "build-web" / "teleco-sc991.html"
wasm = pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else out.parent / "teleco.wasm"
keys = (root / "keys.js").read_text()
html = (root / "index.template.html").read_text()
html = html.replace("/*KEYS_JS*/", keys).replace("/*WASM_B64*/", base64.b64encode(wasm.read_bytes()).decode())
out.write_text(html)
print(f"wrote {out} ({len(html)//1024} KiB)")
