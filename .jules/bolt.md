## 2024-07-07 - Run-Length Encoding for TFT Rendering
**Learning:** Rendering solid blocks or pixel art sprites on hardware displays using pixel-by-pixel `fillRect` calls has significant TFT SPI driver overhead.
**Action:** Use run-length encoding by grouping contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce overhead.
