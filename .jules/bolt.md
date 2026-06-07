## 2024-05-24 - Run-length encoding for hardware display rendering
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays (e.g., using `CreatureRenderer`), using a naive pixel-by-pixel `fillRect` loop causes massive SPI driver overhead. Grouping contiguous horizontal pixels of the same color into a single `fillRect` call significantly reduces TFT SPI driver overhead compared to pixel-by-pixel rendering.
**Action:** When implementing sprite or shape rendering on TFT displays, use run-length encoding (RLE) to group contiguous pixels.
