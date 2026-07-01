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
#include "EvolutionTree.h"
#include <SD.h>

#define SD_FRAME_MAX 128                 // max sprite dimension supported
#define SD_TRANSPARENT565 0x07FF         // cyan = transparent (see sprite_forge.py)

// Procedural fallback silhouettes per archetype (used when SD art is absent).
// One of the existing 1-bit ghost shapes is reused to give each line a distinct
// look; multicolor SD frames (Phase 3) layer on top of this when available.
static const uint8_t* ARCH_FALLBACK[ARCHETYPE_COUNT] = {
    GHOST_GHOST,       // GENESIS
    GHOST_DAEMON,      // JAMMER  (demonic)
    GHOST_POLTERGEIST, // SPAMMER (chaos)
    GHOST_BYTE,        // SNIFFER (techy)
    GHOST_WRAITH       // STRIKER (flowing/electric)
};

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
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    
    // LED Init
    ledFxMode = LED_SOLID;
    ledFxColor = 0;
    ledFxStart = 0;
    ledFxDuration = 0;

    // SD sprite cache buffer (up to 128x128 RGB565).
    frameBuf = (uint16_t*)malloc(SD_FRAME_MAX * SD_FRAME_MAX * sizeof(uint16_t));
}

// Map the current animation to one of the 4 expression frames on disk
// (0=neutral, 1=blink, 2=happy, 3=angry).
uint8_t CreatureRenderer::frameKindForAnim() {
    switch (currentAnim) {
        case ANIM_HAPPY:
        case ANIM_EATING:
            return 2;
        case ANIM_ATTACK:
        case ANIM_HACKING:
        case ANIM_CRITICAL:
            return 3;
        case ANIM_IDLE:
            return ((millis() / 4000) % 5 == 0) ? 1 : 0; // occasional blink
        default:
            return 0;
    }
}

// Load /sprites/<prefix>/<prefix>_s<N>_<frame>.bin into frameBuf.
bool CreatureRenderer::loadSDFrame(uint8_t archetype, uint8_t baseN, uint8_t frameKind) {
    if (!frameBuf || archetype >= ARCHETYPE_COUNT) return false;
    const char* pfx = ARCHETYPE_PREFIX[archetype];
    char path[48];
    snprintf(path, sizeof(path), "/sprites/%s/%s_s%u_%u.bin", pfx, pfx, baseN, frameKind);

    File f = SD.open(path, FILE_READ);
    if (!f) return false;

    uint8_t hdr[4];
    if (f.read(hdr, 4) != 4) { f.close(); return false; }
    int w = hdr[0] | (hdr[1] << 8);
    int h = hdr[2] | (hdr[3] << 8);
    if (w <= 0 || h <= 0 || w > SD_FRAME_MAX || h > SD_FRAME_MAX) { f.close(); return false; }

    size_t bytes = (size_t)w * h * 2;
    size_t got = f.read((uint8_t*)frameBuf, bytes);
    f.close();
    if (got != bytes) return false;

    frameW = w;
    frameH = h;
    return true;
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
/* // === LED CONTROL ===
    unsigned long now = millis();
    uint16_t finalLedColor = currentColor; // Changed from params.color to currentColor
    
    if (ledFxMode != LED_SOLID) {
        if (ledFxDuration > 0 && (now - ledFxStart > ledFxDuration)) {
            ledFxMode = LED_SOLID; // Auto reset
        } else {
            // Apply FX
            switch (ledFxMode) {
                case LED_STROBE:
                    if ((now / 50) % 2 == 0) finalLedColor = 0; // Blink fast
                    else finalLedColor = ledFxColor;
                    break;
                case LED_PULSE: {
                    float val = (sin(now / 200.0f) + 1.0f) * 0.5f; // 0.0 to 1.0
                    // Mix black and color
                    uint8_t r = ((ledFxColor >> 11) & 0x1F) * val;
                    uint8_t g = ((ledFxColor >> 5) & 0x3F) * val;
                    uint8_t b = (ledFxColor & 0x1F) * val;
                    finalLedColor = (r << 11) | (g << 5) | b;
                    break; 
                }
                case LED_RAINBOW: {
                    // Simple hue cycle
                    uint8_t hue = (now / 10) % 255;
                    // Mock rainbow (simplified)
                    if (hue < 85) finalLedColor = TFT_RED;
                    else if (hue < 170) finalLedColor = TFT_GREEN;
                    else finalLedColor = TFT_BLUE; 
                    // Better rainbow would be HSL to RGB565 but this is enough
                    break;
                }
                case LED_OFF:
                    finalLedColor = 0;
                    break;
                default:
                    break;
            }
        }
    }
    
    // Convert 565 to PWM (approx)
    uint8_t led_r = (finalLedColor >> 11) & 0x1F; // Renamed to avoid conflict
    uint8_t led_g = (finalLedColor >> 5) & 0x3F; // Renamed to avoid conflict
    uint8_t led_b = finalLedColor & 0x1F; // Renamed to avoid conflict
    
    // Scale to 0-255
    analogWrite(4, led_r * 8);   // R
    analogWrite(16, led_g * 4);  // G
    analogWrite(17, led_b * 8);  // B */
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

// Phase 0: the "Data Core .ENC" — a dark pulsing polygonal core with falling
// matrix characters and a virtual lock, before the archetype is assigned.
void CreatureRenderer::drawEgg(int centerX, int centerY) {
    unsigned long now = millis();
    updateParticles();
    spr->fillSprite(TFT_BLACK);

    int cx = 64, cy = 64;
    float pulse = (sin(now / 300.0) + 1.0) * 0.5;     // 0..1
    int r = 26 + (int)(pulse * 5);
    uint16_t core = 0x10A2;                            // dark blue-grey
    uint16_t edge = 0x07FF;                            // cyan

    // Filled diamond (polygonal data core)
    for (int dy = -r; dy <= r; dy++) {
        int w = r - abs(dy);
        spr->drawFastHLine(cx - w, cy + dy, 2 * w, core);
    }
    spr->drawLine(cx, cy - r, cx + r, cy, edge);
    spr->drawLine(cx + r, cy, cx, cy + r, edge);
    spr->drawLine(cx, cy + r, cx - r, cy, edge);
    spr->drawLine(cx - r, cy, cx, cy - r, edge);

    // Virtual lock glyph blinking
    spr->setTextColor(((now / 400) % 2) ? 0xFFFF : 0xFD20);
    spr->drawString(".ENC", cx - 12, cy - 4);

    // Falling matrix characters
    for (int i = 0; i < 7; i++) {
        int x = (i * 19 + (int)(now / 50)) % 128;
        int y = (i * 27 + (int)(now / 18)) % 128;
        char ch = (((now / 7) + i) % 2) ? '1' : '0';
        spr->drawChar(x, y, ch, 0x07E0, 0, 1);
    }

    drawParticles();
    currentColor = edge;

    int sx = centerX - 64, sy = centerY - 64;
    if (sx < 0) sx = 0;
    if (sx > 240 - 128) sx = 240 - 128;
    if (sy < 0) sy = 0;
    if (sy > 320 - 128) sy = 320 - 128;
    spr->pushSprite(sx, sy);
}

void CreatureRenderer::draw(int centerX, int centerY, uint8_t archetype, uint8_t stage,
                            bool aggressive, int mood) {
    unsigned long now = millis();
    if (archetype >= ARCHETYPE_COUNT) archetype = 0;
    if (stage < 1) stage = 1;
    if (stage > MAX_STAGE) stage = MAX_STAGE;
    
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
    
    // No blinking for stability
    // if (!isBlinking && (now - lastBlinkTime > 4000)) { isBlinking = true; lastBlinkTime = now; }
    // else if (isBlinking && (now - lastBlinkTime > 150)) { isBlinking = false; lastBlinkTime = now; }
    
    spr->fillSprite(TFT_BLACK);
    
    const uint8_t* sprite = ARCH_FALLBACK[archetype];
    uint16_t color = ARCHETYPE_COLORS[archetype];
    uint16_t glowColor = ARCHETYPE_GLOW[archetype];

    if (aggressive) {
        color = 0xF800; // attacking -> red regardless of line
    }

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

    // Try a multicolor SD frame for this archetype/base/expression (cached by
    // key so we only hit the card when something changes).
    bool haveArt = false;
    if (sdReady && frameBuf) {
        uint8_t baseN = BASE_STAGE_NUM[baseImageForStage(stage)];
        uint8_t fk = frameKindForAnim();
        if (!(frameValid && cachedArch == archetype && cachedBaseN == baseN && cachedFrame == fk)) {
            frameValid = loadSDFrame(archetype, baseN, fk);
            cachedArch = archetype; cachedBaseN = baseN; cachedFrame = fk;
        }
        haveArt = frameValid;
    }

    int scale = 4;
    // For 128x128 sprite: Ghost (96px) centered at (16, 16)
    // animOffset adds movement within the sprite
    int drawX = 16 + (int)posX + animOffsetX;  // Base: center 96px ghost in 128px
    int drawY = 16 + (int)posY + animOffsetY;
    
    // Store screen coordinates for particles (sprite will be pushed at centerX-64)
    lastDrawX = centerX - 64 + drawX;
    lastDrawY = centerY - 64 + drawY;
    
    if (haveArt) {
        // Blit the multicolor SD frame (cyan transparent), centered + animated.
        int ax = (128 - frameW) / 2 + (int)posX + animOffsetX;
        int ay = (128 - frameH) / 2 + (int)posY + animOffsetY;
        spr->setSwapBytes(false); // frameBuf is native-endian RGB565
        spr->pushImage(ax, ay, frameW, frameH, frameBuf, (uint16_t)SD_TRANSPARENT565);
        spr->setSwapBytes(true);
    } else {
        // Procedural 1-bit fallback silhouette.
        // [Bolt] Run-length encoding optimization to reduce TFT SPI driver overhead
        for (int row = 0; row < 24; row++) {
            uint32_t rowData = (pgm_read_byte(&sprite[row * 3]) << 16) |
                               (pgm_read_byte(&sprite[row * 3 + 1]) << 8) |
                               (pgm_read_byte(&sprite[row * 3 + 2]));

            int runStart = -1;
            for (int col = 0; col <= 24; col++) {
                bool isSet = (col < 24) && ((rowData >> (23 - col)) & 0x01);
                if (isSet) {
                    if (runStart == -1) runStart = col;
                } else {
                    if (runStart != -1) {
                        int px = drawX + runStart * scale;
                        int py = drawY + row * scale;
                        spr->fillRect(px, py, (col - runStart) * scale, scale, color);
                        runStart = -1;
                    }
                }
            }
        }
    }

    // Stage aura: concentric glow rings whose count grows with the stage (and
    // pops at milestones 4/8/10). This is the additive "power/aura" growth.
    int auraRings = stage / 3; // 0..3
    if (isMilestone(stage)) auraRings++;
    int cx = drawX + 48, cy = drawY + 48;
    if (auraRings > 0) {
        for (int a = 1; a <= auraRings; a++) {
            spr->drawCircle(cx, cy, 48 + a * 5, glowColor);
        }
    }
    // Even stages reuse the previous base + add an extra overlay accent ring.
    if (hasOverlay(stage)) {
        int rr = 44 + (int)(sin(now / 250.0) * 4);
        spr->drawCircle(cx, cy, rr, color);
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

    // Push sprite at ghost position (sprite follows ghost)
    int spriteX = centerX - 64;  // Center 128px sprite on ghost X
    int spriteY = centerY - 64;  // Center 128px sprite on ghost Y
    
    // Clamp to screen bounds
    if (spriteX < 0) spriteX = 0;
    if (spriteX > 240 - 128) spriteX = 240 - 128;
    if (spriteY < 0) spriteY = 0;
    if (spriteY > 320 - 128) spriteY = 320 - 128;
    
    spr->pushSprite(spriteX, spriteY);
}

void CreatureRenderer::drawSprite(int x, int y, const uint8_t* sprite, uint16_t color, int scale) {
    // [Bolt] Run-length encoding optimization to reduce TFT SPI driver overhead
    for (int row = 0; row < 24; row++) {
        uint32_t rowData = (pgm_read_byte(&sprite[row * 3]) << 16) |
                           (pgm_read_byte(&sprite[row * 3 + 1]) << 8) |
                           (pgm_read_byte(&sprite[row * 3 + 2]));
        
        int runStart = -1;
        for (int col = 0; col <= 24; col++) {
            bool isSet = (col < 24) && ((rowData >> (23 - col)) & 0x01);
            if (isSet) {
                if (runStart == -1) runStart = col;
            } else {
                if (runStart != -1) {
                    spr->fillRect(x + runStart * scale, y + row * scale, (col - runStart) * scale, scale, color);
                    runStart = -1;
                }
            }
        }
    }
}
