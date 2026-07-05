## 2024-07-06 - RLE Sprite Rendering
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays using CreatureRenderer, pixel-by-pixel rendering via fillRect incurs significant TFT SPI driver overhead. Grouping contiguous horizontal pixels of the same color into a single fillRect call (Run-Length Encoding) drastically reduces this overhead.
**Action:** Use RLE for 1-bit or simple sprite rendering to group contiguous horizontal pixels, flushing trailing pixels by extending column loops by one.
