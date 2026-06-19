## 2024-05-24 - Hardware Sprite Rendering Overhead
**Learning:** Pixel-by-pixel rendering of solid blocks or sprites on TFT SPI displays causes massive overhead due to individual driver calls (`fillRect` for each scaled pixel).
**Action:** Use run-length encoding by grouping contiguous horizontal pixels of the same color into a single `fillRect` call.
