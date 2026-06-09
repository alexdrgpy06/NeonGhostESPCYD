## 2024-05-15 - Run-length Encoding for Sprite Rendering
**Learning:** Drawing hardware pixel art pixel-by-pixel causes immense TFT SPI driver overhead.
**Action:** Use run-length encoding by grouping contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce TFT SPI driver overhead.
