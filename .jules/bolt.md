## 2024-05-24 - Hardware Display SPI Rendering Optimization
**Learning:** Pixel-by-pixel rendering causes significant TFT SPI overhead.
**Action:** Always group contiguous horizontal pixels and draw them with a single `fillRect` call using Run-Length Encoding (RLE) to dramatically reduce driver latency on ESP32 CYD.
