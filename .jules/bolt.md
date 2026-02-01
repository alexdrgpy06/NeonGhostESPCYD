## 2026-02-01 - Render Loop RLE Optimization
**Learning:** In embedded graphics (TFT_eSPI), minimizing the number of SPI transactions (or `setWindow` calls) is crucial. A naive bitmap drawer calls `fillRect` for every pixel, which is very inefficient.
**Action:** Implemented Run-Length Encoding (RLE) in the rendering loop to batch consecutive horizontal pixels into a single `fillRect` call. This reduces overhead significantly for the 24x24 sprites used in Neon Ghost.
