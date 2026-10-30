#!/usr/bin/env python3
"""Convert the simulator's PBM frame to a scaled LCD-style PNG (for docs and review)."""
import sys
from PIL import Image

def main(src, dst, scale=4):
    toks = open(src).read().split()
    w, h = int(toks[1]), int(toks[2])
    bits = "".join(toks[3:])
    img = Image.new("RGB", (w * scale + 8, h * scale + 8), (24, 24, 24))
    bg, fg = (176, 192, 168), (24, 32, 24)
    px = img.load()
    for y in range(h):
        for x in range(w):
            c = fg if bits[y * w + x] == "1" else bg
            for dy in range(scale):
                for dx in range(scale):
                    # slight dot gap so the dot matrix is visible
                    px[4 + x * scale + dx, 4 + y * scale + dy] = c if (dx < scale - 1 and dy < scale - 1) else bg if c == bg else (110, 124, 104)
    img.save(dst)

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], int(sys.argv[3]) if len(sys.argv) > 3 else 4)
