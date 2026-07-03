## 2025-01-20 - TFT Rendering Overhead
**Learning:** Pixel-by-pixel rendering using `fillRect` calls incurs significant SPI driver overhead on the ESP32 for hardware displays.
**Action:** Use Run-Length Encoding (RLE) to group contiguous horizontal pixels of the same color into a single `fillRect` call, reducing the number of SPI transactions.
