## 2025-02-14 - Run-Length Encoding for TFT Rendering
**Learning:** Pixel-by-pixel rendering (`fillRect` for every pixel) on TFT displays introduces significant SPI driver overhead, creating a bottleneck in `CreatureRenderer`.
**Action:** Group contiguous horizontal pixels of the same color into a single `fillRect` call using run-length encoding to drastically reduce SPI transaction overhead.
