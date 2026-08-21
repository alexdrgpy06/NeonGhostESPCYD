## 2026-08-21 - [Rendering Optimization]
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays, use run-length encoding by grouping contiguous horizontal pixels of the same color into a single fillRect call to significantly reduce TFT SPI driver overhead compared to pixel-by-pixel rendering.
**Action:** Apply RLE for contiguous block rendering.
