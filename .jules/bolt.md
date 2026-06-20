## 2025-02-18 - RLE Optimization for 1-bit Fallback Sprites
**Learning:** The TFT display driver overhead from individual pixel-by-pixel rendering of solid blocks or 1-bit sprites is significantly high due to SPI driver calls.
**Action:** When rendering solid blocks or pixel art sprites on hardware displays, use run-length encoding (RLE) to group contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce TFT SPI driver overhead compared to pixel-by-pixel rendering.
