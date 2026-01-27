#ifndef PET_STATS_H
#define PET_STATS_H

#include <Arduino.h>

#define STAGE_COUNT 15

// =============================================================================
// EVOLUTION STAGES (15 total)
// =============================================================================
enum EvolutionStage {
    STAGE_SPARK = 0,      // Lvl 1-2
    STAGE_BYTE,           // Lvl 3-4
    STAGE_GHOST,          // Lvl 5-6
    STAGE_SPECTER,        // Lvl 7-9
    STAGE_PHANTOM,        // Lvl 10-12
    STAGE_WRAITH,         // Lvl 13-15
    STAGE_SHADE,          // Lvl 16-18
    STAGE_REVENANT,       // Lvl 19-22
    STAGE_BANSHEE,        // Lvl 23-26
    STAGE_LICH,           // Lvl 27-30
    STAGE_POLTERGEIST,    // Lvl 31-35
    STAGE_VOID,           // Lvl 36-40
    STAGE_NIGHTMARE,      // Lvl 41-47
    STAGE_REAPER,         // Lvl 48-55
    STAGE_DAEMON          // Lvl 56+
};

// =============================================================================
// PET STATS STRUCTURE
// =============================================================================
struct PetStats {
    // Core stats
    uint8_t hp;          // Health (0-100)
    uint8_t mp;          // Energy/Mana (0-100)
    
    // Progression
    int level;
    int xp;
    int xpMax;
    EvolutionStage stage;
    
    // Timestamps
    unsigned long lastMPDecay;
    unsigned long lastHPCheck;
    unsigned long lastActivity;
    unsigned long lastAutoAttack;
    
    // State
    bool isSleeping;
    bool isDead;
};

// =============================================================================
// STAT MANAGER CLASS
// =============================================================================
class PetStatsManager {
public:
    PetStats stats;
    
    void init();
    void load();
    void save();
    void reset();
    
    // Updates
    void update();
    void decayMP();
    void checkHP();
    
    // Actions
    void feed(int amount = 20);
    void rest();
    void wake();
    void revive();
    
    // Auto-attack system
    bool tryAutoAttack();  // Returns true if attack triggered
    int getAttackMPCost();
    
    // XP & Leveling
    void addXP(int amount);
    void addHP(int amount);
    void addMP(int amount);
    bool checkLevelUp();
    void evolve();
    
    // Getters
    EvolutionStage getStage();
    int getStageFromLevel(int level);
    const char* getStageName();
    const char* getAttackName();
    uint8_t getAttackType();  // 0=WiFi, 1=BT, 2=All
    uint16_t getStageColor();
    
private:
    static const unsigned long MP_DECAY_RATE = 90000;   // 1.5 min
    static const unsigned long HP_CHECK_RATE = 10000;   // 10 sec (faster drain when hungry)
    static const unsigned long AUTO_ATTACK_RATE = 5000; // 5 sec
};

// XP Rewards
#define XP_HANDSHAKE     100
#define XP_PMKID         150
#define XP_DEAUTH        25
#define XP_NEW_NETWORK   10
#define XP_BLE_DEVICE    8
#define XP_PROBE         3
#define XP_FEED          20
#define XP_PLAY          15
#define XP_AUTO_ATTACK   5

// HP/MP rewards
#define HP_CAPTURE       5
#define HP_HANDSHAKE     15
#define MP_CAPTURE       10
#define MP_HANDSHAKE     20
#define MP_FEED          15

#endif
