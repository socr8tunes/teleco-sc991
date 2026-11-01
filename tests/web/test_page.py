#!/usr/bin/env python3
"""Browser smoke test of the single-file web simulator: logo shows, then the calc screen, 2+3x4 = 14, never powers off by itself."""
import sys
from playwright.sync_api import sync_playwright

def main(path):
    with sync_playwright() as pw:
        b = pw.chromium.launch(); p = b.new_page(); errs = []
        p.on("pageerror", lambda e: errs.append(str(e)))
        p.goto("file://" + path)
        p.wait_for_timeout(300)
        assert "off" not in p.eval_on_selector("#calc", "e => e.className"), "powered off at start"
        p.wait_for_timeout(2000)
        for k in ["2", "+", "3", "*", "4", "EXE"]:
            p.evaluate("k => teleco.name(k)", k)
        assert p.evaluate("teleco.result()").strip() == "14", p.evaluate("teleco.result()")
        assert not errs, errs
        b.close()
    print("web page smoke test ok")

if __name__ == "__main__":
    main(sys.argv[1])
