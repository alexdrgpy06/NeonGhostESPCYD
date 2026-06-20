## 2024-06-16 - Run-Length Encoding (RLE) for Sprite Rendering
**Learning:** Drawing large numbers of individual pixels on TFT displays using single `fillRect` calls causes excessive SPI overhead and micro-delays that degrade framerate.
**Action:** When rendering pixel art sprites or solid blocks, group contiguous horizontal pixels of the same color and draw them with a single `fillRect` to minimize hardware driver overhead.
