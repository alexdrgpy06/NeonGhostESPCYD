## 2026-02-07 - [Sprite Rendering Optimization]
**Learning:** Pixel-by-pixel rendering on SPI displays via `fillRect(1x1)` is extremely inefficient due to transaction overhead.
**Action:** Always batch horizontal pixel runs into larger `fillRect` calls (RLE) or use a frame buffer if RAM permits. On CYD (ESP32), this yields ~80% reduction in draw calls for typical sprites.
