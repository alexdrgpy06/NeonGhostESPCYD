## 2024-05-15 - Optimize hardware sprite rendering with Run-Length Encoding
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays (e.g., using `CreatureRenderer`), sending `fillRect` for every single pixel causes high TFT SPI driver overhead. Grouping contiguous horizontal pixels of the same color into a single `fillRect` call (Run-Length Encoding) significantly reduces SPI calls and speeds up rendering compared to pixel-by-pixel.
**Action:** Always use Run-Length Encoding (RLE) to group contiguous pixels when drawing pixel art over TFT SPI interfaces.
