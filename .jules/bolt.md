## 2026-07-07 - TFT SPI Run-Length Encoding Optimization
**Learning:** Pixel-by-pixel rendering via `fillRect` causes severe SPI overhead on hardware displays due to per-call driver commands. Grouping contiguous horizontal pixels reduces this overhead significantly.
**Action:** When rendering pixel art on hardware displays, implement Run-Length Encoding (RLE) to batch contiguous X-axis pixels into single `fillRect` calls. Cleanly flush trailing pixels by extending the column loop by one.
