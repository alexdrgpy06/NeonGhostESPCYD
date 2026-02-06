## 2024-05-22 - [Bitmap Rendering Overhead]
**Learning:** Drawing bitmaps pixel-by-pixel using `fillRect` (1x1) incurs massive overhead due to repeated function calls and coordinate setup, especially for scaled sprites.
**Action:** Use Run-Length Encoding (RLE) to batch horizontal pixel runs into single `fillRect` calls. Also, explicitly check for and skip empty rows to avoid inner loop processing entirely.
