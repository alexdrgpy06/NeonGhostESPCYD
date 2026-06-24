## 2024-06-23 - TFT SPI Rendering Overhead
**Learning:** Rendering pixel art on hardware displays by plotting individual pixels (e.g., using a nested loop and `fillRect` for each pixel) causes excessive TFT SPI driver overhead.
**Action:** When rendering solid blocks or pixel art sprites (e.g., using `CreatureRenderer`), use run-length encoding (RLE) by grouping contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce overhead.
