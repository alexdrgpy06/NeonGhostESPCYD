## 2026-02-05 - [Sprite Rendering RLE Optimization]
**Learning:** `TFT_eSPI` sprite operations like `fillRect` have measurable overhead per call due to clipping and color setup. When drawing 1-bit bitmaps, pixel-by-pixel rendering (576 calls for 24x24) is significantly slower than batched RLE rendering (approx 24-50 calls).
**Action:** Always look for opportunities to batch adjacent pixel operations when rendering bitmap data manually, especially on embedded systems where CPU cycles per frame matter for 30fps smooth animation.
