# Sprite assets (editable source)

These are the **editable** placeholder sprites for the pet's evolution lines.
Replace them with your own art following the design in
[`../docs/EVOLUTION_CANVAS.md`](../docs/EVOLUTION_CANVAS.md). A ready-to-use
**image-generation prompt for every BMP** (by line/stage/expression) is in
[`../docs/SPRITE_PROMPTS.md`](../docs/SPRITE_PROMPTS.md).

## Layout

```
assets/sprites/<prefix>/<prefix>_s<N>_<frame>.bmp
```

- `<prefix>`: `gen` GENESIS · `jam` JAMMER · `spa` SPAMMER · `sni` SNIFFER · `str` STRIKER
- `<N>`: base stage `1, 3, 5, 7, 9, 10`. Even stages (2,4,6,8) **reuse the
  previous odd base** in firmware and only add a procedural overlay — so you only
  draw 6 bases per line.
- `<frame>`: `0` neutral · `1` blink · `2` happy · `3` angry

So per line: 6 bases × 4 expressions = 24 sprites (120 total).

## Transparency

The sprites are **24-bit BMP** and **cyan (RGB 0,255,255) is transparent**. Paint
your character on the cyan background; anything left cyan shows the screen
behind it.

> ⚠️ GENESIS uses cyan accents — for *real* cyan details use a near-cyan such as
> RGB (0,254,255) so they aren't keyed out.

## Build for the SD card

```bash
# Convert every .bmp here into the firmware's RGB565 .bin tree
python3 ../tools/sprite_forge.py build . ../sd_root

# Copy the result to the SD card root, giving you:
#   SD:/sprites/<prefix>/<prefix>_s<N>_<frame>.bin
cp -R ../sd_root/sprites /path/to/sdcard/
```

The firmware loads these `.bin` frames automatically when an SD card is present;
without a card it falls back to procedural silhouettes.
