## 2026-06-30 - TFT SPI Overhead Reduction in Pixel Art
**Learning:** Rendering pixel art pixel-by-pixel using `fillRect` for each solid pixel creates significant SPI driver overhead on the TFT screen.
**Action:** Group contiguous horizontal pixels of the same color into a single `fillRect` call using Run-Length Encoding (RLE) with a clean flush by extending loop bounds, reducing total SPI transactions.
