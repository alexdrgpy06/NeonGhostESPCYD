## 2024-08-16 - RLE for Sprite Rendering
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays, pixel-by-pixel rendering causes severe TFT SPI driver overhead. Grouping contiguous horizontal pixels of the same color into a single fillRect call (Run-Length Encoding) significantly reduces this overhead.
**Action:** Use an inner loop that extends by one bound (e.g., col <= width) and evaluates out-of-bounds pixels as false to safely flush any trailing contiguous pixels into a single block.
