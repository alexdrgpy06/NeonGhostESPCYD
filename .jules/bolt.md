## 2024-10-24 - Hardware TFT SPI Rendering Run-Length Optimization
**Learning:** Rendering pixel art scaled up on hardware displays via TFT SPI is bottlenecked by individual `fillRect` calls per pixel due to command/data overhead.
**Action:** Group contiguous horizontal pixels of the same color into a single `fillRect` call (run-length encoding) to significantly reduce SPI driver overhead.
