/**
 * ╔═══════════════════════════════════════════════════════════════╗
 * ║              CREATURE RENDERER MODULE v7.0                    ║
 * ║   Handles sprite drawing, animations and particle effects     ║
 * ║                                                               ║
 * ║   Features: 240px Full-Width Buffer, RGB LED Sync,            ║
 * ║   11 Animation States, Dynamic Event Colors                   ║
 * ║                                                               ║
 * ║                     by Alex R.                                ║
 * ╚═══════════════════════════════════════════════════════════════╝
 */
#include "CreatureRenderer.h"
#include "GhostSprites.h"

// Ghost-following 128x128 sprite - efficient and flicker-free


CreatureRenderer::CreatureRenderer(TFT_eSPI *tft) {
    _tft = tft;
    spr = new TFT_eSprite(tft);
    spr->createSprite(128, 128); // Ghost-following sprite (32KB, memory efficient)
    spr->setSwapBytes(true);
    
    lastBlinkTime = 0;
    isBlinking = false;
    
    currentAnim = ANIM_IDLE;
    animStart = 0;
    animDuration = 0;
    
    posX = 0;
    posY = 0;
    lastDrawX = 120; // Default center
    lastDrawY = 64;
    targetX = 0;
    targetY = 0;
    velX = 0;
    velY = 0;
    lastMove = 0;

    // Optimization State Init
    prevSpriteX = -999;
    prevSpriteY = -999;
    prevInternalDrawX = -999;
    prevInternalDrawY = -999;
    prevColor = 0;
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    
    // LED Init
    ledFxMode = LED_SOLID;
    ledFxColor = 0;
    ledFxStart = 0;
    ledFxDuration = 0;
}

int CreatureRenderer::getStageFromLevel(int level) {
    if (level <= 2) return 0;        // SPARK
    else if (level <= 4) return 1;   // BYTE
    else if (level <= 6) return 2;   // GHOST
    else if (level <= 9) return 3;   // SPECTER
    else if (level <= 12) return 4;  // PHANTOM
    else if (level <= 15) return 5;  // WRAITH
    else if (level <= 18) return 6;  // SHADE
    else if (level <= 22) return 7;  // REVENANT
    else if (level <= 26) return 8;  // BANSHEE
    else if (level <= 30) return 9;  // LICH
    else if (level <= 35) return 10; // POLTERGEIST
    else if (level <= 40) return 11; // VOID
    else if (level <= 47) return 12; // NIGHTMARE
    else if (level <= 55) return 13; // REAPER
    else return 14;                  // DAEMON
}

void CreatureRenderer::triggerAnimation(AnimState anim, int duration) {
    currentAnim = anim;
    animStart = millis();
    animDuration = duration;
    
    if (anim == ANIM_HAPPY) {
        emitParticles(lastDrawX + 48, lastDrawY + 48, 8, 0x07E0, PART_SPARK);
    } else if (anim == ANIM_EVOLVING) {
        emitParticles(lastDrawX + 48, lastDrawY + 48, 12, 0xF81F, PART_RING);
    } else if (anim == ANIM_ATTACK) {
        emitParticles(lastDrawX + 48, lastDrawY + 48, 6, 0xFFE0, PART_SPARK);
    }
}

void CreatureRenderer::updatePosition() {
    unsigned long now = millis();
    float dt = (now - lastMove) / 1000.0f;
    lastMove = now;
    
    if (dt > 0.1f) dt = 0.1f;
    
    float speed = (currentAnim == ANIM_SLEEPING) ? 0.5f : 3.0f;
    
    if (random(0, 100) < 3) {
        targetX = random(-30, 31);
        targetY = random(-20, 21);
    }
    
    float dx = targetX - posX;
    float dy = targetY - posY;
    
    posX += dx * speed * dt;
    posY += dy * speed * dt;
    
    if (posX < -35) posX = -35;
    if (posX > 35) posX = 35;
    if (posY < -25) posY = -25;
    if (posY > 25) posY = 25;
}

void CreatureRenderer::emitParticles(int x, int y, int count, uint16_t color, ParticleType type) {
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (!particles[i].active) {
            particles[i].active = true;
            particles[i].x = x;
            particles[i].y = y;
            particles[i].color = color;
            particles[i].type = type;
            particles[i].life = 255;
            particles[i].maxLife = 255;
            
            if (type == PART_RING) {
                particles[i].vx = 0;
                particles[i].vy = 0;
            } else if (type == PART_ZZZ) {
                particles[i].vx = random(-10, 10) / 10.0f;
                particles[i].vy = -1.5f - random(0, 10) / 10.0f;
                particles[i].ch = 'Z';
            } else {
                float angle = random(0, 628) / 100.0f;
                float speed = 1.0f + random(0, 30) / 10.0f;
                particles[i].vx = cos(angle) * speed;
                particles[i].vy = sin(angle) * speed;
            }
            count--;
        }
    }
}

void CreatureRenderer::updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            
            if (particles[i].type == PART_RING) {
                particles[i].life -= 3;
            } else {
                particles[i].life -= 5;
            }
            
            if (particles[i].life <= 0 || particles[i].x < 0 || particles[i].x > 128 ||
                particles[i].y < 0 || particles[i].y > 128) {
                particles[i].active = false;
            }
        }
    }
}

void CreatureRenderer::drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            uint8_t alpha = particles[i].life;
            uint16_t c = particles[i].color;
            
            uint8_t r = ((c >> 11) & 0x1F) * alpha / 255;
            uint8_t g = ((c >> 5) & 0x3F) * alpha / 255;
            uint8_t b = (c & 0x1F) * alpha / 255;
            uint16_t fadedColor = (r << 11) | (g << 5) | b;
            
            switch (particles[i].type) {
                case PART_DOT:
                    spr->fillCircle((int)particles[i].x, (int)particles[i].y, 2, fadedColor);
                    break;
                case PART_SPARK:
                    spr->drawPixel((int)particles[i].x, (int)particles[i].y, fadedColor);
                    spr->drawPixel((int)particles[i].x + 1, (int)particles[i].y, fadedColor);
                    spr->drawPixel((int)particles[i].x, (int)particles[i].y + 1, fadedColor);
                    break;
                case PART_RING: {
                    int radius = (255 - particles[i].life) / 5;
                    spr->drawCircle((int)particles[i].x, (int)particles[i].y, radius, fadedColor);
                    break;
                }
                case PART_ZZZ:
                    spr->setTextColor(fadedColor);
                    spr->drawChar((int)particles[i].x, (int)particles[i].y, particles[i].ch, fadedColor, 0, 1);
                    break;
            }
        }
    }
}



void CreatureRenderer::drawZZZ(int x, int y, float phase) {
    for (int i = 0; i < 3; i++) {
        int bx = x + 40 + i * 8;
        int by = y - 10 - i * 12 - (int)(sin(phase + i) * 5);
        spr->drawChar(bx, by, 'Z', 0x07FF, 0, 1);
    }
}

// Set LED Effect
void CreatureRenderer::setLedFx(LedMode mode, uint16_t color, int duration) {
    ledFxMode = mode;
    ledFxColor = color;
    ledFxDuration = duration;
    ledFxStart = millis();
}

void CreatureRenderer::draw(int centerX, int centerY, int level, EvolutionStage stage, int mood) {
    unsigned long now = millis();
    
    updatePosition();
    updateParticles();
    
    if (currentAnim != ANIM_IDLE && (now - animStart > (unsigned long)animDuration)) {
        currentAnim = ANIM_IDLE;
    }
    
    int animOffsetX = 0;
    int animOffsetY = 0;
    float animPhase = (now - animStart) / 100.0f;
    float globalPhase = now / 1000.0f;
    
    switch (currentAnim) {
        case ANIM_IDLE: 
            // Dual-frequency breathing for natural feel
            animOffsetY = (int)(sin(now / 600.0) * 4.0 + sin(now / 1500.0) * 2.0); 
            break;
        case ANIM_EATING: 
            animOffsetY = (int)(sin(animPhase * 8) * 6); 
            break;
        case ANIM_HAPPY: 
            // Bouncy jump with decay
            animOffsetY = (int)(abs(sin(animPhase * 4)) * -18); 
            animOffsetX = (int)(sin(animPhase * 2) * 3);
            break;
        case ANIM_ALERT: 
            animOffsetX = (int)(sin(animPhase * 12) * 8); 
            break;
        case ANIM_EVOLVING: 
            // Spiral motion
            animOffsetY = (int)(sin(animPhase * 3) * 10); 
            animOffsetX = (int)(cos(animPhase * 3) * 8); 
            break;
        case ANIM_ATTACK: 
            // Sharp shake
            animOffsetX = (int)(random(-5, 6)); 
            animOffsetY = (int)(random(-3, 4)); 
            break;
        case ANIM_SLEEPING: 
            // Very slow gentle bob
            animOffsetY = (int)(sin(now / 2000.0) * 2.0); 
            if (random(0, 80) < 3) emitParticles(lastDrawX + 60, lastDrawY + 20, 1, 0x07FF, PART_ZZZ); 
            break;
        case ANIM_HACKING: 
            // Glitchy micro-movements
            animOffsetX = (int)(random(-2, 3)); 
            if (random(0, 10) < 2) animOffsetY = random(-2, 3);
            break;
        case ANIM_SCANNING: 
            // Smooth radar sweep
            animOffsetY = (int)(sin(now / 150.0) * 3); 
            break;
        case ANIM_CRITICAL: 
            // Violent shake
            animOffsetX = (int)(random(-10, 11)); 
            animOffsetY = (int)(random(-10, 11)); 
            break;
        case ANIM_DEATH: 
            // Sink down
            animOffsetY = (int)((now - animStart) / 40); 
            if (animOffsetY > 60) animOffsetY = 60; 
            break;
    }
    
    int stageIdx = getStageFromLevel(level);
    const uint8_t* sprite = GHOST_STAGES[stageIdx];
    uint16_t color = STAGE_COLORS[stageIdx];
    uint16_t glowColor = STAGE_GLOW_COLORS[stageIdx];
    
    if (currentAnim == ANIM_EVOLVING) {
        color = STAGE_COLORS[((now / 100) % 7)];
    } else if (currentAnim == ANIM_CRITICAL) {
        if (random(0, 2) == 0) color = 0xF800;
    } else if (currentAnim == ANIM_EATING) {
        color = 0xFD20; // Orange for feeding/events
    } else if (currentAnim == ANIM_ATTACK) {
        color = 0xF800; // Red for attacking/deauth
    } else if (currentAnim == ANIM_HACKING) {
        color = 0x07E0; // Matrix Green
    } else if (currentAnim == ANIM_SCANNING) {
        color = 0x07FF; // Cyan Radar
    } else if (currentAnim == ANIM_DEATH) {
        float fadeAmount = (now - animStart) / (float)animDuration;
        if (fadeAmount > 1.0f) fadeAmount = 1.0f;
        uint8_t grey = 128 - (int)(fadeAmount * 100);
        color = (grey >> 3) << 11 | (grey >> 2) << 5 | (grey >> 3);
    } else if (currentAnim == ANIM_SLEEPING) {
        color = glowColor;
    }
    
    currentColor = color; // Sync for LED
    
    int scale = 4;
    // For 128x128 sprite: Ghost (96px) centered at (16, 16)
    // animOffset adds movement within the sprite
    int drawX = 16 + (int)posX + animOffsetX;  // Base: center 96px ghost in 128px
    int drawY = 16 + (int)posY + animOffsetY;
    
    // Store screen coordinates for particles (sprite will be pushed at centerX-64)
    lastDrawX = centerX - 64 + drawX;
    lastDrawY = centerY - 64 + drawY;
    
    // Push sprite at ghost position (sprite follows ghost)
    int spriteX = centerX - 64;  // Center 128px sprite on ghost X
    int spriteY = centerY - 64;  // Center 128px sprite on ghost Y
    
    // Clamp to screen bounds
    if (spriteX < 0) spriteX = 0;
    if (spriteX > 240 - 128) spriteX = 240 - 128;
    if (spriteY < 0) spriteY = 0;
    if (spriteY > 320 - 128) spriteY = 320 - 128;

    // === LED CONTROL ===
    // Use local color
    uint16_t baseColor = color;
    uint16_t finalLedColor = baseColor; 
    
    // Check effects
    if (ledFxMode != LED_SOLID) {
        if (ledFxDuration > 0 && (millis() - ledFxStart > ledFxDuration)) {
            ledFxMode = LED_SOLID; // Auto reset
        } else {
            // Apply FX (Strobe / Pulse / Rainbow)
            unsigned long t = millis();
            switch (ledFxMode) {
                case LED_STROBE:
                    if ((t / 50) % 2 == 0) finalLedColor = 0; 
                    else finalLedColor = ledFxColor;
                    break;
                case LED_PULSE: {
                    float v = (sin(t / 200.0f) + 1.0f) * 0.5f;
                    uint8_t r = ((ledFxColor >> 11) & 0x1F) * v;
                    uint8_t g = ((ledFxColor >> 5) & 0x3F) * v;
                    uint8_t b = (ledFxColor & 0x1F) * v;
                    finalLedColor = (r << 11) | (g << 5) | b;
                    break; 
                }
                case LED_RAINBOW: {
                    uint8_t h = (t / 10) % 255;
                    // Simple rainbow map
                    if (h < 85) finalLedColor = 0xF800; // Red
                    else if (h < 170) finalLedColor = 0x07E0; // Green
                    else finalLedColor = 0x001F; // Blue
                    break;
                }
                default: break;
            }
        }
    }
    
    // Update currentColor for other uses
    currentColor = finalLedColor;

    // Write to LED Pins (CYD RGB)
    uint8_t lr = (finalLedColor >> 11) & 0x1F;
    uint8_t lg = (finalLedColor >> 5) & 0x3F;
    uint8_t lb = finalLedColor & 0x1F;
    analogWrite(4, lr * 8);
    analogWrite(16, lg * 4);
    analogWrite(17, lb * 8);

    // === OPTIMIZATION CHECK ===
    bool particlesActive = false;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particlesActive = true;
            break;
        }
    }

    // Optimization: Skip drawing if state hasn't changed
    // Only applies during IDLE when no particles are active
    if (currentAnim == ANIM_IDLE && !particlesActive &&
        spriteX == prevSpriteX && spriteY == prevSpriteY &&
        drawX == prevInternalDrawX && drawY == prevInternalDrawY &&
        color == prevColor) {
        // Nothing changed visually, skip SPI transfer
        return;
    }

    // Update state
    prevSpriteX = spriteX;
    prevSpriteY = spriteY;
    prevInternalDrawX = drawX;
    prevInternalDrawY = drawY;
    prevColor = color;
    
    // Store screen coordinates for particles (sprite will be pushed at centerX-64)
    lastDrawX = centerX - 64 + drawX;
    lastDrawY = centerY - 64 + drawY;

    // DRAWING
    spr->fillSprite(TFT_BLACK);

    // SECOND PASS: Main pixels
    for (int row = 0; row < 24; row++) {
        uint32_t rowData = (pgm_read_byte(&sprite[row * 3]) << 16) |
                           (pgm_read_byte(&sprite[row * 3 + 1]) << 8) |
                           (pgm_read_byte(&sprite[row * 3 + 2]));

        for (int col = 0; col < 24; col++) {
            if ((rowData >> (23 - col)) & 0x01) {
                int px = drawX + col * scale;
                int py = drawY + row * scale;
                spr->fillRect(px, py, scale, scale, color);
            }
        }
    }

    drawParticles();

    if (currentAnim == ANIM_SLEEPING) {
        drawZZZ(drawX, drawY, globalPhase);
    }

    if (currentAnim == ANIM_EATING) {
        int packetX = drawX + 90 - (int)(animPhase * 15);
        int packetY = drawY + 40;
        if (packetX > drawX + 50) {
            spr->fillRect(packetX, packetY, 12, 8, 0xFD20);
            spr->drawRect(packetX, packetY, 12, 8, TFT_WHITE);
        }
    }
    
    if (currentAnim == ANIM_HAPPY) {
        for (int i = 0; i < 3; i++) {
            int px = random(0, 128);
            int py = random(0, 50);
            spr->fillCircle(px, py, 2, color);
        }
    }

    if (currentAnim == ANIM_ATTACK) {
        // Simple pulse effect instead of lines
        int spriteCenterX = drawX + 48;
        int spriteCenterY = drawY + 48;
        int pulseSize = (int)(animPhase * 2) % 30;
        spr->drawCircle(spriteCenterX, spriteCenterY, pulseSize + 10, TFT_YELLOW);
        spr->drawCircle(spriteCenterX, spriteCenterY, pulseSize + 20, 0xFD20);
    }

    if (currentAnim == ANIM_CRITICAL) {
        // Simple red tint flash
        // Just skip - the red color change is enough
    }

    spr->pushSprite(spriteX, spriteY);
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
