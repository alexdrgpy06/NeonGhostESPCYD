## 2024-05-24 - Hardware TFT Drawing Bottleneck
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays using TFT_eSPI, drawing pixel-by-pixel (or block-by-block) creates significant SPI driver overhead. Grouping contiguous horizontal pixels into a single `fillRect` call (Run-Length Encoding) drastically reduces the number of SPI transactions.
**Action:** Use Run-Length Encoding (RLE) when rendering 1-bit or uncompressed graphics on embedded displays to minimize SPI overhead.
