## 2024-08-18 - RLE Optimization for SPI Displays
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays using TFT SPI drivers, pixel-by-pixel rendering has massive overhead per draw call. Grouping contiguous horizontal pixels into a single run-length (RLE) `fillRect` drastically reduces this overhead.
**Action:** Always consider horizontal RLE sweeps instead of nested per-pixel loops for 1-bit or paletted bitmaps on embedded hardware.
