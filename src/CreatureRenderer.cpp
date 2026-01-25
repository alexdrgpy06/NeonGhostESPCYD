#include "CreatureRenderer.h"

CreatureRenderer::CreatureRenderer(TFT_eSPI *tft) {
    _tft = tft;
    spr = new TFT_eSprite(tft);
    // Create a sprite big enough for the creature (128x128)
    spr->createSprite(128, 128);
}

void CreatureRenderer::draw(int x, int y, unsigned long seed, int level) {
    // Reset RNG with seed
    randomSeed(seed);
    
    // Clear sprite
    spr->fillSprite(TFT_BLACK);
    
    // Aesthetic Colors
    uint16_t cPrimary = PALETTE[random(0, 5)];
    uint16_t cSecondary = PALETTE[random(0, 5)];
    uint16_t cGlow = TFT_WHITE;
    
    // --- GENERATIVE PIXEL ART (12x12 Grid mirrored to 24x24) ---
    // We simulate a 24x24 pixel grid, scaled up by 'scale' factor.
    int scale = 4; 
    int gridSize = 12; // Half-width
    
    int spriteW = 24 * scale;
    int spriteH = 24 * scale;
    int offsetX = (128 - spriteW) / 2;
    int offsetY = (128 - spriteH) / 2;

    for (int py = 0; py < 24; py++) {
        for (int px = 0; px < 12; px++) {
            // Cellular Automata / Noise Logic
            // Probability increases towards center
            int prob = 40 + (px * 5); 
            if (py > 8 && py < 16) prob += 20; // Dense core
            
            bool pixelOn = (random(0, 100) < prob);
            
            // Hardcoded "Face" area clearing
            if (py >= 8 && py <= 12 && px < 4) pixelOn = false; 

            if (pixelOn) {
                // Color Logic
                uint16_t color = cPrimary;
                if (random(0, 10) > 8) color = cSecondary;
                
                // Draw Left Pixel
                int drawX_L = offsetX + ((11 - px) * scale);
                int drawY = offsetY + (py * scale);
                spr->fillRect(drawX_L, drawY, scale, scale, color);
                
                // Draw Right Pixel (Mirror)
                int drawX_R = offsetX + ((12 + px) * scale);
                spr->fillRect(drawX_R, drawY, scale, scale, color);
            }
        }
    }
    
    // --- EYES (The Soul) ---
    int eyeSize = scale * 2;
    int eyeY = offsetY + (10 * scale);
    int eyeX_L = offsetX + (9 * scale);
    int eyeX_R = offsetX + (13 * scale); // 12 + 1
       
    // Eye Glow
    spr->fillRect(eyeX_L, eyeY, eyeSize, eyeSize, TFT_RED);
    spr->fillRect(eyeX_R, eyeY, eyeSize, eyeSize, TFT_RED);
    
    // --- LEVEL GLITCH ---
    // If level is high, corrupt some pixels
    if (level > 1) {
        for (int i=0; i < level * 5; i++) {
             int rx = random(0, 128);
             int ry = random(0, 128);
             int rw = random(2, 10);
             int rh = random(1, 3);
             spr->fillRect(rx, ry, rw, rh, cSecondary);
        }
    }
    
    // --- ANIMATION (Idle Bob) ---
    // Use millis() to calculate a sine wave offset
    int bobOffset = (int)(sin(millis() / 200.0) * 4.0);
    
    // Push Sprite
    spr->pushSprite(x - 64, y - 64 + bobOffset);
}
