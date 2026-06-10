## 2026-06-09 - RLE Optimization for Hardware Displays
**Learning:** When rendering solid blocks or pixel art sprites on hardware displays (like the CYD's TFT via SPI), sending individual pixel drawing commands (`fillRect` for a 1x1 block) incurs significant driver overhead.
**Action:** Use run-length encoding (RLE) to group contiguous horizontal pixels of the same color into a single `fillRect` call, drastically reducing SPI transaction overhead and improving framerates.
