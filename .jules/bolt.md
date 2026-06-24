## 2025-02-18 - RLE Optimization for SPI Displays
**Learning:** Drawing contiguous solid horizontal pixels one by one via `fillRect` causes immense overhead due to per-call SPI driver communication when rendering pixel art sprites on hardware displays.
**Action:** When rendering pixel art or solid blocks, use Run-Length Encoding (RLE) by grouping contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce SPI driver overhead. Ensure the loop bound is extended by one (`col <= width`) to flush any trailing contiguous sequences.
