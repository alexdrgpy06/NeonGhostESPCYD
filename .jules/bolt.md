## 2023-10-27 - Bitmap Rendering Optimization
**Learning:** Drawing bitmaps pixel-by-pixel using `fillRect` on `TFT_eSprite` incurs significant overhead due to loop and function call frequency, even if in RAM.
**Action:** Use Run-Length Encoding (RLE) to batch horizontal pixel runs into single `fillRect` calls for static bitmaps.
