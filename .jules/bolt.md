## 2024-05-24 - Hardware Rendering Run-Length Encoding
**Learning:** Pixel-by-pixel rendering on hardware displays like the TFT SPI driver introduces significant overhead due to individual function calls and data transfers per pixel.
**Action:** When rendering solid blocks or pixel art sprites (e.g., in `CreatureRenderer`), use run-length encoding by grouping contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce overhead.
