## 2024-08-20 - TFT SPI Render Optimization
**Learning:** Using `fillRect` pixel-by-pixel for sprite rendering adds significant TFT SPI driver overhead compared to grouping contiguous pixels. Hardware displays perform much better with fewer SPI commands.
**Action:** Always implement Run-Length Encoding (RLE) when rendering pixel art or solid blocks to group contiguous horizontal pixels into a single `fillRect` call.
