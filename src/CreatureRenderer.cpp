#include "CreatureRenderer.h"
#include "GhostSprites.h"

CreatureRenderer::CreatureRenderer(TFT_eSPI *tft) {
    _tft = tft;
    spr = new TFT_eSprite(tft);
    spr->createSprite(128, 128);
    spr->setSwapBytes(true);
    
    lastBlinkTime = 0;
    isBlinking = false;
    
    // Animation
    currentAnim = ANIM_IDLE;
    animStart = 0;
    animDuration = 0;
    
    // Position
    posX = 0;
    posY = 0;
    targetX = 0;
    targetY = 0;
    velX = 0;
    velY = 0;
    lastMove = 0;
}

int CreatureRenderer::getStageFromLevel(int level) {
    if (level <= 3) return 0;
    else if (level <= 6) return 1;
    else if (level <= 9) return 2;
    else if (level <= 14) return 3;
    else return 4;
}

void CreatureRenderer::triggerAnimation(AnimState anim, int duration) {
    currentAnim = anim;
    animStart = millis();
    animDuration = duration;
}

void CreatureRenderer::updatePosition() {
    unsigned long now = millis();
    float dt = (now - lastMove) / 1000.0f;
    lastMove = now;
    
    // Limit delta time
    if (dt > 0.1f) dt = 0.1f;
    
    // Random wandering target change (more frequent)
    if (random(0, 100) < 3) {
        targetX = random(-30, 31);  // Larger range
        targetY = random(-20, 21);
    }
    
    // Move towards target (faster)
    float dx = targetX - posX;
    float dy = targetY - posY;
    
    posX += dx * 3.0f * dt;
    posY += dy * 3.0f * dt;
    
    // Clamp to larger bounds
    if (posX < -35) posX = -35;
    if (posX > 35) posX = 35;
    if (posY < -25) posY = -25;
    if (posY > 25) posY = 25;
}

void CreatureRenderer::draw(int centerX, int centerY, int level, EvolutionStage stage, int mood) {
    unsigned long now = millis();
    
    // Update wandering position
    updatePosition();
    
    // Check animation expiry
    if (currentAnim != ANIM_IDLE && (now - animStart > (unsigned long)animDuration)) {
        currentAnim = ANIM_IDLE;
    }
    
    // Calculate animation offsets
    int animOffsetX = 0;
    int animOffsetY = 0;
    float animPhase = (now - animStart) / 100.0f;
    
    switch (currentAnim) {
        case ANIM_IDLE:
            // Gentle bob
            animOffsetY = (int)(sin(now / 500.0) * 5.0);
            break;
            
        case ANIM_EATING:
            // Quick chomp motion
            animOffsetY = (int)(sin(animPhase * 8) * 6);
            break;
            
        case ANIM_HAPPY:
            // Excited bouncing
            animOffsetY = (int)(abs(sin(animPhase * 5)) * -15);
            break;
            
        case ANIM_ALERT:
            // Side-to-side shake
            animOffsetX = (int)(sin(animPhase * 10) * 10);
            break;
            
        case ANIM_EVOLVING:
            // Spin/glitch effect
            animOffsetY = (int)(sin(animPhase * 4) * 8);
            animOffsetX = (int)(cos(animPhase * 3) * 5);
            break;

        case ANIM_ATTACK:
            // Violent shake
            animOffsetX = (int)(random(-4, 5));
            animOffsetY = (int)(random(-4, 5));
            break;
    }
    
    // Blinking
    if (!isBlinking && (now - lastBlinkTime > 4000)) {
        isBlinking = true;
        lastBlinkTime = now;
    } else if (isBlinking && (now - lastBlinkTime > 150)) {
        isBlinking = false;
        lastBlinkTime = now;
    }
    
    // Clear sprite
    spr->fillSprite(TFT_BLACK);
    
    // Get sprite and color
    int stageIdx = getStageFromLevel(level);
    const uint8_t* sprite = GHOST_STAGES[stageIdx];
    uint16_t color = STAGE_COLORS[stageIdx];
    
    // Evolution animation: cycle colors
    if (currentAnim == ANIM_EVOLVING) {
        int colorIdx = ((now / 100) % 5);
        color = STAGE_COLORS[colorIdx];
    }
    
    // Dim glow color
    uint16_t glowColor = (color >> 2) & 0x39E7;
    
    // Calculate final position
    int scale = 4;
    int baseX = (128 - 24 * scale) / 2;
    int baseY = (128 - 24 * scale) / 2;
    
    int drawX = baseX + (int)posX + animOffsetX;
    int drawY = baseY + (int)posY + animOffsetY;
    
    // FIRST PASS: Glow
    for (int row = 0; row < 24; row++) {
        uint32_t rowData = (pgm_read_byte(&sprite[row * 3]) << 16) |
                           (pgm_read_byte(&sprite[row * 3 + 1]) << 8) |
                           (pgm_read_byte(&sprite[row * 3 + 2]));
        
        for (int col = 0; col < 24; col++) {
            if ((rowData >> (23 - col)) & 0x01) {
                int px = drawX + col * scale;
                int py = drawY + row * scale;
                spr->fillRect(px - 2, py - 2, scale + 4, scale + 4, glowColor);
            }
        }
    }
    
    // SECOND PASS: Main pixels
    for (int row = 0; row < 24; row++) {
        // Glitch effect during evolution
        int rowShift = 0;
        if (currentAnim == ANIM_EVOLVING && random(0, 100) < 20) {
            rowShift = random(-3, 4) * scale;
        }
        
        uint32_t rowData = (pgm_read_byte(&sprite[row * 3]) << 16) |
                           (pgm_read_byte(&sprite[row * 3 + 1]) << 8) |
                           (pgm_read_byte(&sprite[row * 3 + 2]));
        
        for (int col = 0; col < 24; col++) {
            if ((rowData >> (23 - col)) & 0x01) {
                int px = drawX + col * scale + rowShift;
                int py = drawY + row * scale;
                spr->fillRect(px, py, scale, scale, color);
            }
        }
    }
    
    // Eating animation: draw "packet" being eaten
    if (currentAnim == ANIM_EATING) {
        int packetX = drawX + 90 - (int)(animPhase * 15);
        int packetY = drawY + 40;
        if (packetX > drawX + 50) {
            spr->fillRect(packetX, packetY, 12, 8, 0xFD20); // Orange packet
            spr->drawRect(packetX, packetY, 12, 8, TFT_WHITE);
        }
    }
    
    // Happy animation: particles
    if (currentAnim == ANIM_HAPPY) {
        for (int i = 0; i < 3; i++) {
            int px = random(0, 128);
            int py = random(0, 50);
            spr->fillCircle(px, py, 2, color);
        }
    }

    // Attack animation: Lightning bolts
    if (currentAnim == ANIM_ATTACK) {
        // Draw random lightning bolts from center
        int centerX = drawX + 48; // 24*scale/2
        int centerY = drawY + 48;
        for (int i = 0; i < 6; i++) {
            int len = random(30, 60);
            float angle = random(0, 628) / 100.0;
            int ex = centerX + (int)(cos(angle) * len);
            int ey = centerY + (int)(sin(angle) * len);
            spr->drawLine(centerX, centerY, ex, ey, TFT_YELLOW);
            // Draw a second segment
            int ex2 = ex + random(-10, 11);
            int ey2 = ey + random(-10, 11);
            spr->drawLine(ex, ey, ex2, ey2, TFT_WHITE);
        }
        // Flash effect
        if (random(0, 2) == 0) {
            spr->drawRect(0, 0, 128, 128, TFT_WHITE);
        }
    }
    
    // Push to screen
    spr->pushSprite(centerX - 64, centerY - 64);
}

void CreatureRenderer::drawSprite(int x, int y, const uint8_t* sprite, uint16_t color, int scale) {
    for (int row = 0; row < 24; row++) {
        uint32_t rowData = (pgm_read_byte(&sprite[row * 3]) << 16) |
                           (pgm_read_byte(&sprite[row * 3 + 1]) << 8) |
                           (pgm_read_byte(&sprite[row * 3 + 2]));
        
        for (int col = 0; col < 24; col++) {
            if ((rowData >> (23 - col)) & 0x01) {
                spr->fillRect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}
