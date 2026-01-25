#include "CreatureRenderer.h"

CreatureRenderer::CreatureRenderer(TFT_eSPI *tft) { _tft = tft; }

void CreatureRenderer::draw(int x, int y, unsigned long seed, int level) {
  // IMPORTANTE: Resetea el RNG localmente usando la semilla del bicho
  randomSeed(seed);

  uint16_t colorPrincipal = PALETTE[random(0, 5)];

  // 1. EL CUERPO (Formas aleatorias superpuestas)
  int w = random(40, 80);
  int h = random(40, 80);
  int bodyType = random(0, 3);

// Config constants
#define C_FONDO TFT_BLACK
#define C_GLITCH TFT_MAGENTA

  _tft->fillRect(x - w / 2, y - h / 2, w, h,
                 C_FONDO); // Limpiar área previa

  if (bodyType == 0) { // Bloque sólido
    _tft->fillRect(x - w / 2, y - h / 2, w, h, colorPrincipal);
    _tft->drawRect(x - w / 2, y - h / 2, w, h, TFT_WHITE);
  } else if (bodyType == 1) { // Hexágono simulado (corte de esquinas)
    _tft->fillRect(x - w / 2, y - h / 2, w, h, colorPrincipal);
    _tft->fillTriangle(x - w / 2, y - h / 2, x - w / 2 + 10, y - h / 2,
                       x - w / 2, y - h / 2 + 10, C_FONDO); // Corte sup izq
  } else { // Estilo "Invader" (Pixelado grueso)
    for (int i = 0; i < 6; i++) {
      int px = random(x - 40, x + 40);
      int py = random(y - 40, y + 40);
      _tft->fillRect(px, py, random(5, 15), random(5, 15), colorPrincipal);
    }
  }

  // 2. LOS OJOS (La ventana del alma digital)
  int eyeType = random(0, 4);
  int eyeY = y - random(0, 10);
  _tft->setTextColor(TFT_BLACK);

  if (eyeType == 0) { // Cíclope
    _tft->fillRect(x - 10, eyeY, 20, 10, TFT_BLACK);
    _tft->fillRect(x - 2, eyeY + 2, 4, 4, TFT_RED);
  } else if (eyeType == 1) { // Dos líneas
    _tft->fillRect(x - 20, eyeY, 15, 5, TFT_BLACK);
    _tft->fillRect(x + 5, eyeY, 15, 5, TFT_BLACK);
  } else if (eyeType == 2) { // Asimétricos (Glitch)
    _tft->fillCircle(x - 15, eyeY, random(3, 8), TFT_BLACK);
    _tft->fillRect(x + 5, eyeY - 5, 10, 10, TFT_BLACK);
  }

  // 3. ACCESORIOS (Antenas, Auras)
  if (random(0, 2) == 1) { // Antena
    _tft->drawLine(x, y - h / 2, x, y - h / 2 - 20, colorPrincipal);
    _tft->fillCircle(x, y - h / 2 - 20, 3, TFT_RED);
  }

  // 4. EFECTO GLITCH (Ruido visual alrededor)
  for (int i = 0; i < level + 2; i++) { // Más nivel = Más ruido
    int gx = x + random(-60, 60);
    int gy = y + random(-60, 60);
    _tft->drawPixel(gx, gy, C_GLITCH);
  }
}
