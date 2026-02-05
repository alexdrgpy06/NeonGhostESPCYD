## 2024-05-23 - [Optimization] RLE for Sprite Rendering
**Learning:** On embedded displays with SPI interfaces (like the CYD's ST7789/ILI9341), minimizing the number of transaction calls is often more critical than CPU cycles. Drawing a sprite pixel-by-pixel (many small `fillRect` calls) causes massive overhead due to setup/teardown of SPI transactions.
**Action:** Use Run-Length Encoding (RLE) to batch adjacent pixels into a single `fillRect` call. This creates "strips" of pixels, reducing SPI calls by 3-5x for typical sprite art. Always check for completely empty rows to skip them entirely.
