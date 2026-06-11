## 2024-05-24 - Hardware TFT Rendering Run-Length Encoding Optimization
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays (e.g., using `CreatureRenderer`), use run-length encoding by grouping contiguous horizontal pixels of the same color into a single `fillRect` call to significantly reduce TFT SPI driver overhead compared to pixel-by-pixel rendering.
**Action:** Always refactor iterative `fillRect` or `drawPixel` calls over bitmapped data to scan for contiguous spans and issue a single block draw command.
