#include "CreatureRenderer.h"
#include "assets.h"

CreatureRenderer::CreatureRenderer(TFT_eSPI *tft) {
  _tft = tft;
  // Initialize Sprite (64x64 canvas for creature)
  spr = new TFT_eSprite(tft);
  spr->setColorDepth(16);
  spr->createSprite(64, 64);
  spr->setSwapBytes(true); // Important for RGB565 assets
}

void CreatureRenderer::draw(int x, int y, unsigned long seed, int level) {
  unsigned long now = millis();

  // 1. Clear Sprite (Black background matches main BG)
  spr->fillSprite(TFT_BLACK);

  // 2. Calculate Animation (Bobbing)
  int y_offset = (int)(sin(now / 300.0) * 4.0);

  // 3. Draw Ghost Sprite (Centered in 64x64 sprite)
  // img_ghost is 32x32. Center is (16, 16 + offset)
  int gx = 16;
  int gy = 16 + y_offset;

  // Push the asset to the sprite
  // Note: assets.h arrays are const uint16_t* so we cast
  spr->pushImage(gx, gy, 32, 32, (uint16_t*)img_ghost);

  // 4. Glitch Effect (Randomly offset horizontal lines)
  // "Cyberpunk Hologram" effect
  if (random(20) == 0) { // 5% chance per frame
      int line = random(64);
      int shift = random(5) - 2;
      spr->scroll(shift, 0); // Scroll whole sprite? No, this scrolls everything.
      // Better: Draw a random scanline
      spr->drawFastHLine(0, line, 64, TFT_CYAN);
  }

  // 5. Level Indicator (Aura)
  if (level > 1) {
      // Simple rotating pixels or glowing eyes
      int eye_color = (level > 5) ? TFT_RED : TFT_GREEN;
      // Eyes are at roughly +10, +10 inside the 32x32 sprite
      // Let's just tint the sprite or add circles
      // Drawing circles around the ghost
      spr->drawCircle(32, 32 + y_offset, 20 + (level * 2), 0x0040); // Dark Green ring
  }

  // 6. Push to Screen
  // x,y passed are the CENTER coordinates on screen
  // We push top-left of sprite, so x - 32, y - 32
  spr->pushSprite(x - 32, y - 32);
}
