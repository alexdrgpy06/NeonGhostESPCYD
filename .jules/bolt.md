## 2026-08-22 - Optimize 1-bit sprite rendering with Run-Length Encoding
**Learning:** Pixel-by-pixel rendering of solid blocks issues multiple small drawing commands, increasing TFT SPI driver overhead. Grouping contiguous horizontal pixels into a single fillRect call significantly reduces this overhead.
**Action:** Use Run-Length Encoding (RLE) when rendering pixel art sprites to hardware displays to minimize SPI transactions.
