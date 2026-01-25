#ifndef CREATURE_RENDERER_H
#define CREATURE_RENDERER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

class CreatureRenderer {
public:
  CreatureRenderer(TFT_eSPI *tft);
  void draw(int x, int y, unsigned long seed, int level);

private:
  TFT_eSPI *_tft;
  // Cyberpunk Palette
  const uint16_t PALETTE[5] = {TFT_GREEN, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW,
                               0xFD20}; // 0xFD20 is Orange
};

#endif
