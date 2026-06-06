## 2024-05-18 - Hardware Display Rendering RLE
**Learning:** Grouping contiguous horizontal pixels into a single `fillRect` call (Run-Length Encoding) significantly reduces TFT SPI driver overhead compared to pixel-by-pixel rendering when drawing pixel art sprites on hardware displays.
**Action:** Always prefer run-length encoding (drawing horizontal lines/blocks) over individual pixel drawing when rendering 2D tile or sprite data to TFT/SPI screens to avoid SPI transaction overhead per pixel.
