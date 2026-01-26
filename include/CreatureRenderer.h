#ifndef CREATURE_RENDERER_H
#define CREATURE_RENDERER_H

#include <TFT_eSPI.h>

// Animation states
enum AnimState {
    ANIM_IDLE,      // Normal floating
    ANIM_EATING,    // Mouth open, chomping
    ANIM_HAPPY,     // Bouncing up and down
    ANIM_ALERT,     // Shaking side to side
    ANIM_EVOLVING,  // Spinning/glitching
    ANIM_ATTACK     // Shockwave/Zap
};

class CreatureRenderer {
public:
    enum EvolutionStage { BABY, TEEN, ADULT };
    
    CreatureRenderer(TFT_eSPI *tft);
    
    // Main draw function
    void draw(int centerX, int centerY, int level, EvolutionStage stage, int mood = 0);
    
    // Trigger animations
    void triggerAnimation(AnimState anim, int duration = 1000);
    
    // Get stage from level
    int getStageFromLevel(int level);

private:
    TFT_eSPI *_tft;
    TFT_eSprite *spr;
    
    // Animation state
    AnimState currentAnim;
    unsigned long animStart;
    int animDuration;
    
    // Position (for wandering)
    float posX, posY;      // Current position offset
    float targetX, targetY; // Target position
    float velX, velY;       // Velocity
    unsigned long lastMove;
    
    // Blinking
    unsigned long lastBlinkTime;
    bool isBlinking;
    
    // Helpers
    void updatePosition();
    void drawSprite(int x, int y, const uint8_t* sprite, uint16_t color, int scale);
};

#endif
