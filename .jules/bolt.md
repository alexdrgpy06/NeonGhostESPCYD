## 2025-05-24 - Hardware Sprite Rendering Bottleneck
**Learning:** Pixel-by-pixel rendering of 1-bit sprites using individual fillRect calls causes excessive SPI overhead, acting as a major bottleneck on embedded TFTs.
**Action:** Use Run-Length Encoding (RLE) by iterating up to col <= width to group contiguous horizontal pixels into a single, wider fillRect call, flushing gracefully when out of bounds.
