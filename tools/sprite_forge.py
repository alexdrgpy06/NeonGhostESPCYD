#!/usr/bin/env python3
"""
sprite_forge.py — asset pipeline for NeonGhost (CYD / ESP32).

Converts PNG images into the raw RGB565 ``.bin`` frames the firmware loads from
the SD card, following the layout:

    /sprites/<prefix>/<prefix>_s<n>_<frame>.bin

where ``<prefix>`` is one of: gen, jam, spa, sni, str (GENESIS/JAMMER/SPAMMER/
SNIFFER/STRIKER). ``<n>`` is the base stage (1, 3, 5, 7, 9, 10 — even stages
reuse the previous odd base) and ``<frame>`` is an expression frame
(0=neutral, 1=blink, 2=happy, 3=angry).

Each .bin is: 2-byte little-endian width, 2-byte height, then width*height
RGB565 little-endian pixels. Transparent pixels (alpha < 128) are written as the
magic colour 0xF81F (magenta) which the renderer treats as transparent.

Usage:
    # one image -> one frame bin
    python3 tools/sprite_forge.py convert in.png out.bin [--size 96]

    # scaffold an empty SD tree with placeholder bins for all archetypes
    python3 tools/sprite_forge.py scaffold ./sd_root [--size 96]

The 'scaffold' command does not need Pillow; 'convert' needs `pip install pillow`.
"""
import argparse
import os
import struct
import sys

ARCHETYPES = ["gen", "jam", "spa", "sni", "str"]
BASES = [1, 3, 5, 7, 9, 10]
FRAMES = 4
TRANSPARENT = 0xF81F

# Per-archetype placeholder tint (RGB565) used when scaffolding without art.
TINTS = {
    "gen": 0xFFFF,
    "jam": 0xF800,
    "spa": 0xF81F,
    "sni": 0x041F,
    "str": 0xFFE0,
}


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def write_bin(path, width, height, pixels565):
    with open(path, "wb") as f:
        f.write(struct.pack("<HH", width, height))
        for p in pixels565:
            f.write(struct.pack("<H", p))


def convert(in_png, out_bin, size):
    try:
        from PIL import Image
    except ImportError:
        sys.exit("Pillow required: pip install pillow")
    img = Image.open(in_png).convert("RGBA")
    if size:
        img = img.resize((size, size), Image.NEAREST)
    w, h = img.size
    px = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = img.getpixel((x, y))
            px.append(TRANSPARENT if a < 128 else rgb565(r, g, b))
    write_bin(out_bin, w, h, px)
    print(f"wrote {out_bin} ({w}x{h})")


def placeholder_frame(size, tint, frame):
    """A simple tinted blob placeholder so the tree is runnable before real art."""
    px = [TRANSPARENT] * (size * size)
    cx, cy, rad = size // 2, size // 2, size // 2 - 2
    # frame index gently shifts the body so frames differ visibly
    cy += (frame - 1) * 2
    for y in range(size):
        for x in range(size):
            if (x - cx) ** 2 + (y - cy) ** 2 <= rad * rad:
                px[y * size + x] = tint
    return px


def scaffold(root, size):
    for arch in ARCHETYPES:
        d = os.path.join(root, "sprites", arch)
        os.makedirs(d, exist_ok=True)
        for n in BASES:
            for fr in range(FRAMES):
                px = placeholder_frame(size, TINTS[arch], fr)
                write_bin(os.path.join(d, f"{arch}_s{n}_{fr}.bin"), size, size, px)
    total = len(ARCHETYPES) * len(BASES) * FRAMES
    print(f"scaffolded {total} placeholder frames under {root}/sprites/")


def main():
    ap = argparse.ArgumentParser(description="NeonGhost sprite pipeline")
    sub = ap.add_subparsers(dest="cmd", required=True)

    c = sub.add_parser("convert", help="PNG -> RGB565 .bin")
    c.add_argument("input")
    c.add_argument("output")
    c.add_argument("--size", type=int, default=96)

    s = sub.add_parser("scaffold", help="create placeholder SD tree")
    s.add_argument("root")
    s.add_argument("--size", type=int, default=96)

    args = ap.parse_args()
    if args.cmd == "convert":
        convert(args.input, args.output, args.size)
    elif args.cmd == "scaffold":
        scaffold(args.root, args.size)


if __name__ == "__main__":
    main()
