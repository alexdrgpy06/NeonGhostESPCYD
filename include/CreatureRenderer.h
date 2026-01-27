#ifndef CREATURE_RENDERER_H
#define CREATURE_RENDERER_H

#include <TFT_eSPI.h>

// Animation states
enum AnimState {
    ANIM_IDLE,
    ANIM_EATING,
    ANIM_HAPPY,
    ANIM_ALERT,
    ANIM_EVOLVING,
    ANIM_ATTACK,
    ANIM_SLEEPING,
    ANIM_HACKING,
    ANIM_SCANNING,
    ANIM_CRITICAL,
    ANIM_DEATH
};

// Particle types
enum ParticleType {
    PART_DOT,
    PART_CHAR,
    PART_RING,
    PART_SPARK,
    PART_ZZZ
};

struct Particle {
    float x, y;
    float vx, vy;
    uint16_t color;
    uint8_t life;
    uint8_t maxLife;
    ParticleType type;
    char ch;
    bool active;
};

#define MAX_PARTICLES 24

class CreatureRenderer {
public:
    enum EvolutionStage { 
        SPARK, BYTE, GHOST, SPECTER, PHANTOM, WRAITH, 
        SHADE, REVENANT, BANSHEE, LICH, POLTERGEIST, 
        VOID, NIGHTMARE, REAPER, DAEMON,
        GLITCH, CYPHER, OMEGA, AKIRA, SINGULARITY
    };
    
    CreatureRenderer(TFT_eSPI *tft);
    
    // Main draw function - draws directly to TFT at screen coordinates
    void draw(int centerX, int centerY, int level, EvolutionStage stage, int mood = 0);
    
    void triggerAnimation(AnimState anim, int duration = 1000);
    int getStageFromLevel(int level);
    void emitParticles(int x, int y, int count, uint16_t color, ParticleType type);
    void updateParticles();
    void drawParticles();
    
    // LED FX
    enum LedMode {
        LED_SOLID,
        LED_PULSE,
        LED_STROBE,
        LED_RAINBOW,
        LED_OFF
    };
    
    void setLedFx(LedMode mode, uint16_t color, int duration = 0);
    bool isAnimating() { return currentAnim != ANIM_IDLE; } // Helper for ability check
    void drawScanlines();
    
    AnimState currentAnim;
    uint16_t currentColor; // Expose for LED control

private:
    TFT_eSPI *_tft;
    TFT_eSprite *spr;
    
    unsigned long animStart;
    int animDuration;
    
    float posX, posY;
    int lastDrawX, lastDrawY;
    float targetX, targetY;
    float velX, velY;
    unsigned long lastMove;
    
    // LED State
    LedMode ledFxMode;
    uint16_t ledFxColor;
    unsigned long ledFxStart;
    int ledFxDuration;
    
    unsigned long lastBlinkTime;
    bool isBlinking;
    
    Particle particles[MAX_PARTICLES];
    
    void updatePosition();
    void drawSprite(int x, int y, const uint8_t* sprite, uint16_t color, int scale);
    void drawMatrixRain(int x, int y);
    void drawRadarPulse(int centerX, int centerY, float phase);
    void drawZZZ(int x, int y, float phase);
    void drawGlitchOverlay();
};

#endif
