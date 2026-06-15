## 2024-06-16 - TFT Rendering Bottleneck
 **Learning:** Pixel-by-pixel rendering on hardware displays like TFTs introduces significant SPI driver overhead, creating a bottleneck.
 **Action:** Apply run-length encoding (RLE) to group contiguous horizontal pixels into a single `fillRect` call to significantly reduce TFT SPI driver overhead compared to pixel-by-pixel rendering.
