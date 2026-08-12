## 2024-05-24 - Hardware TFT RLE Optimization
 **Learning:** Rendering solid blocks or pixel art sprites on hardware displays using pixel-by-pixel fillRect causes high SPI driver overhead.
 **Action:** Use Run-Length Encoding (RLE) by grouping contiguous horizontal pixels of the same color into a single fillRect call, cleanly flushing trailing pixels by extending column loops by one out-of-bounds.
