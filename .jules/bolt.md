## 2024-06-21 - Hardware Display RLE Optimization
**Learning:** Rendering solid blocks or pixel art sprites pixel-by-pixel on hardware displays (TFT SPI) incurs massive driver and bus overhead due to repeated `fillRect` calls.
**Action:** Always implement run-length encoding (RLE) by grouping contiguous horizontal pixels into a single `fillRect` call to minimize TFT driver overhead. Handle trailing pixels by extending the bounds of the column loop by one.
