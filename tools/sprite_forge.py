#!/usr/bin/env python3
"""
sprite_forge.py — asset pipeline for NeonGhost (CYD / ESP32), loki-style.

WORKFLOW
    1. Edit the placeholder **BMP** files (24-bit). Paint your art on the
       **CYAN background** (RGB 0,255,255) — cyan = transparent.
    2. Run `build` to convert the whole BMP tree into the firmware's SD format
       (`.bin`, RGB565), where cyan pixels become the transparent sentinel.
    3. Copy the produced `/sprites` tree to the SD card root.

LAYOUT
    sprites/<prefix>/<prefix>_s<N>_<frame>.bmp   (you edit these)
    sprites/<prefix>/<prefix>_s<N>_<frame>.bin   (firmware loads these)

    <prefix> ∈ gen, jam, spa, sni, str  (GENESIS/JAMMER/SPAMMER/SNIFFER/STRIKER)
    <N>      ∈ 1,3,5,7,9,10  (base stage; even stages reuse the previous odd base)
    <frame>  ∈ 0=neutral, 1=blink, 2=happy, 3=angry

.bin FORMAT
    little-endian: uint16 width, uint16 height, then width*height RGB565 pixels.
    Cyan (RGB565 0x07FF) is the transparent colour the renderer skips.

COMMANDS
    placeholders <dir> [--size 96]      generate the editable BMP tree
    convert <in.(bmp|png)> <out.bin> [--size]   one image -> one .bin
    build <src_dir> <sd_dir> [--size]   convert every .bmp in the tree -> .bin

Pure-Python 24-bit BMP read/write (no Pillow needed). PNG input needs Pillow.

NOTE: cyan is the transparency key, so avoid pure cyan 0x07FF inside GENESIS art
(use a near-cyan like 0x07FE for real cyan details).
"""
import argparse
import os
import struct
import sys

ARCHETYPES = ["gen", "jam", "spa", "sni", "str"]
BASES = [1, 3, 5, 7, 9, 10]
FRAMES = 4
FRAME_NAMES = ["neutral", "blink", "happy", "angry"]

CYAN = (0, 255, 255)            # transparency key (RGB)
TRANSPARENT565 = 0x07FF         # cyan in RGB565 — the firmware skips this

# Per-archetype placeholder tint (RGB) so the blobs are distinguishable.
TINTS = {
    "gen": (240, 240, 255),   # white-ish
    "jam": (220, 30, 30),     # red
    "spa": (230, 30, 230),    # magenta
    "sni": (40, 90, 230),     # blue
    "str": (240, 220, 30),    # yellow
}


def rgb565(r, g, b):
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


# ----------------------------------------------------------------------------
# 24-bit BMP I/O (pure Python)
# ----------------------------------------------------------------------------
def write_bmp24(path, w, h, rows):
    """rows: h lists (top->bottom) of (r,g,b) tuples."""
    row_size = (w * 3 + 3) & ~3
    pad = row_size - w * 3
    data_size = row_size * h
    with open(path, "wb") as f:
        f.write(b"BM")
        f.write(struct.pack("<IHHI", 54 + data_size, 0, 0, 54))
        f.write(struct.pack("<IiiHHIIiiII", 40, w, h, 1, 24, 0, data_size,
                            2835, 2835, 0, 0))
        for y in range(h - 1, -1, -1):  # BMP is bottom-up
            for (r, g, b) in rows[y]:
                f.write(bytes((b, g, r)))
            f.write(b"\x00" * pad)


def read_bmp24(path):
    with open(path, "rb") as f:
        data = f.read()
    if data[:2] != b"BM":
        raise ValueError(f"{path}: not a BMP")
    pixoff = struct.unpack("<I", data[10:14])[0]
    w = struct.unpack("<i", data[18:22])[0]
    h = struct.unpack("<i", data[22:26])[0]
    bpp = struct.unpack("<H", data[28:30])[0]
    if bpp != 24:
        raise ValueError(f"{path}: only 24-bit BMP supported (got {bpp}-bit). "
                         "Re-save as 24-bit BMP, or use a PNG.")
    top_down = h < 0
    h = abs(h)
    row_size = (w * 3 + 3) & ~3
    rows = []
    for ry in range(h):
        y = ry if top_down else (h - 1 - ry)
        off = pixoff + y * row_size
        row = []
        for x in range(w):
            b, g, r = data[off + x * 3], data[off + x * 3 + 1], data[off + x * 3 + 2]
            row.append((r, g, b))
        rows.append(row)
    return w, h, rows


def read_image(path, size):
    """Return (w,h,rows) for a BMP (native) or any Pillow-readable image."""
    if path.lower().endswith(".bmp"):
        return read_bmp24(path)
    try:
        from PIL import Image
    except ImportError:
        sys.exit("Pillow required for non-BMP input: pip install pillow")
    img = Image.open(path).convert("RGB")
    if size:
        img = img.resize((size, size), Image.NEAREST)
    w, h = img.size
    rows = [[img.getpixel((x, y)) for x in range(w)] for y in range(h)]
    return w, h, rows


# ----------------------------------------------------------------------------
# Conversion to firmware .bin
# ----------------------------------------------------------------------------
def to_bin(in_path, out_bin, size):
    w, h, rows = read_image(in_path, size)
    with open(out_bin, "wb") as f:
        f.write(struct.pack("<HH", w, h))
        for y in range(h):
            for (r, g, b) in rows[y]:
                # Snap near-cyan to the exact transparent sentinel.
                if r < 40 and g > 200 and b > 200:
                    f.write(struct.pack("<H", TRANSPARENT565))
                else:
                    f.write(struct.pack("<H", rgb565(r, g, b)))
    print(f"wrote {out_bin} ({w}x{h})")


def build(src_dir, sd_dir, size):
    n = 0
    for root, _, files in os.walk(src_dir):
        for fn in files:
            if not fn.lower().endswith(".bmp"):
                continue
            rel = os.path.relpath(os.path.join(root, fn), src_dir)
            out = os.path.join(sd_dir, os.path.splitext(rel)[0] + ".bin")
            os.makedirs(os.path.dirname(out), exist_ok=True)
            to_bin(os.path.join(root, fn), out, size)
            n += 1
    print(f"built {n} frames into {sd_dir}/")


# ----------------------------------------------------------------------------
# Placeholder BMP generation
# ----------------------------------------------------------------------------
def placeholder_rows(size, tint, base_idx, frame):
    """Cyan canvas + tinted blob; small markers encode base/frame for clarity."""
    rows = [[CYAN for _ in range(size)] for _ in range(size)]
    cx, cy = size // 2, size // 2 + (frame - 1) * 2
    rad = size // 2 - 4
    for y in range(size):
        for x in range(size):
            if (x - cx) ** 2 + (y - cy) ** 2 <= rad * rad:
                rows[y][x] = tint
    # base markers: filled squares along the top (one per base index+1)
    for i in range(base_idx + 1):
        for y in range(2, 6):
            for x in range(2 + i * 5, 5 + i * 5):
                if x < size:
                    rows[y][x] = (0, 0, 0)
    # frame markers: squares along the bottom
    for i in range(frame + 1):
        for y in range(size - 6, size - 2):
            for x in range(2 + i * 5, 5 + i * 5):
                if x < size:
                    rows[y][x] = (0, 0, 0)
    return rows


def placeholders(root, size):
    for arch in ARCHETYPES:
        d = os.path.join(root, "sprites", arch)
        os.makedirs(d, exist_ok=True)
        for bi, n in enumerate(BASES):
            for fr in range(FRAMES):
                rows = placeholder_rows(size, TINTS[arch], bi, fr)
                write_bmp24(os.path.join(d, f"{arch}_s{n}_{fr}.bmp"), size, size, rows)
    total = len(ARCHETYPES) * len(BASES) * FRAMES
    print(f"wrote {total} placeholder BMPs under {root}/sprites/ "
          f"({size}x{size}, cyan = transparent)")


def main():
    ap = argparse.ArgumentParser(description="NeonGhost sprite pipeline (loki-style BMP -> SD .bin)")
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("placeholders", help="generate editable BMP tree (cyan bg)")
    p.add_argument("root")
    p.add_argument("--size", type=int, default=96)

    c = sub.add_parser("convert", help="one image (bmp/png) -> one .bin")
    c.add_argument("input")
    c.add_argument("output")
    c.add_argument("--size", type=int, default=0)

    b = sub.add_parser("build", help="convert a whole .bmp tree -> SD .bin tree")
    b.add_argument("src")
    b.add_argument("sd")
    b.add_argument("--size", type=int, default=0)

    args = ap.parse_args()
    if args.cmd == "placeholders":
        placeholders(args.root, args.size)
    elif args.cmd == "convert":
        to_bin(args.input, args.output, args.size)
    elif args.cmd == "build":
        build(args.src, args.sd, args.size)


if __name__ == "__main__":
    main()
