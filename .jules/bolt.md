## 2024-05-24 - Hardware Rendering Optimization
**Learning:** Rendering pixel-by-pixel using `fillRect` on hardware TFT displays introduces significant SPI driver overhead. Grouping contiguous horizontal pixels into a single `fillRect` call (Run-Length Encoding) provides a substantial performance boost.
**Action:** When implementing rendering loops for pixel art sprites, extend the bounds of the column loop by one to flush trailing pixels and evaluate the pixel status as false when indexing out-of-bounds to group horizontal runs.
