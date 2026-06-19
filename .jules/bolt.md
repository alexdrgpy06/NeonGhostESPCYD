## 2026-06-14 - Optimize TFT pixel rendering with run-length encoding
**Learning:** Rendering complex pixel art sprites by individually drawing each pixel with `fillRect` introduces significant TFT SPI driver overhead, leading to slower screen updates and potential bottlenecks in the main UI rendering loop.
**Action:** When rendering pixel art or solid blocks on hardware displays, always group contiguous horizontal pixels of the same color into a single `fillRect` call using run-length encoding.
