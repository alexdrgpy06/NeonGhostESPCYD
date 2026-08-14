## 2024-05-24 - TFT Rendering Bottlenecks
**Learning:** Pixel-by-pixel rendering via `fillRect` for 1-bit sprites on the CYD TFT display creates immense SPI overhead.
**Action:** Use Run-Length Encoding (RLE) to batch contiguous horizontal pixels into a single `fillRect` call to drastically reduce SPI transactions.
