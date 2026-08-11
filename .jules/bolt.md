## 2026-08-11 - Run-Length Encoding for TFT Rendering
**Learning:** Rendering solid blocks pixel-by-pixel using TFT SPI driver causes massive overhead.
**Action:** Use Run-Length Encoding (RLE) to group contiguous horizontal pixels into single fillRect calls, significantly reducing driver overhead.
