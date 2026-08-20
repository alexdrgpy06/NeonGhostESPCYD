## 2024-10-24 - TFT Rendering Optimization
**Learning:** Pixel-by-pixel rendering causes massive TFT SPI driver overhead.
**Action:** Use run-length encoding (RLE) to group contiguous horizontal pixels into a single fillRect call.
