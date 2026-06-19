#ifndef PET_STATS_H
#define PET_STATS_H

#include <Arduino.h>
#include "EvolutionTree.h"

// =============================================================================
// PET STATS — Digimon-style branching model (5 archetypes x 10 stages + NG+)
// See EvolutionTree.h for the archetype/stage/power data model.
// =============================================================================
struct PetStats {
    // Core vitals
    uint8_t hp;          // Health (0-100)
    uint8_t mp;          // Energy/Mana (0-100)

    // Progression
    int level;
    int xp;
    int xpMax;

    // Evolution position
    Archetype archetype; // current style line (0..4)
    uint8_t   stage;     // 1..10 inside the line
    uint8_t   baseImage; // 0..3 (derived from stage milestones; cached)

    // Branching: what the pet does most steers the route
    uint32_t affinity[ROUTE_COUNT];

    // New Game Plus: mastery grows with use and persists across line jumps
    uint16_t powerMastery[POWER_COUNT];

    // Lifetime counters ("what it did")
    uint32_t totalAttacks;
    uint32_t devicesFound;
    uint32_t lineJumps;
    uint32_t powerupsUnlocked; // potency bumps reached at milestones
    uint16_t deaths;
    uint16_t revives;
    uint16_t ageBoots;         // power cycles survived

    // Timestamps
    unsigned long lastMPDecay;
    unsigned long lastHPCheck;
    unsigned long lastActivity;
    unsigned long lastAutoAttack;

    // State
    bool isSleeping;
    bool isDead;
    bool hatched;        // Phase 0: false = encrypted egg, archetype undetermined
};

// =============================================================================
// STAT MANAGER
// =============================================================================
class PetStatsManager {
public:
    PetStats stats;

    // Event flags raised by evolve(), consumed (and cleared) by the UI loop.
    bool justEvolved = false; // stage advanced
    bool justJumped  = false; // archetype changed (line jump)

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

    // Auto-attack pacing
    bool tryAutoAttack();
    int  getAttackMPCost();

    // Progression
    void addXP(int amount);
    void addHP(int amount);
    void addMP(int amount);
    bool checkLevelUp();
    void evolve();

    // Branching / NG+
    void addAffinity(Route r, uint32_t n);
    void gainMastery(uint8_t powerId, uint16_t n);
    uint16_t masteryOf(uint8_t powerId);
    Route dominant();

    // Effective MP cost of a power (home cheaper, foreign pricier, mastery cuts).
    int powerMpCost(uint8_t powerId);

    // Phase 0: hatch the encrypted egg, assigning the initial archetype from
    // local radio/hardware entropy.
    void hatchEgg();
    bool isEgg() { return !stats.hatched; }

    // Getters
    Archetype   getArchetype();
    uint8_t     getStage();
    const char* getArchetypeName();
    const char* getPhaseName();
    uint16_t    getArchetypeColor();
    uint16_t    getArchetypeGlow();

private:
    static const unsigned long MP_DECAY_RATE   = 90000; // 1.5 min
    static const unsigned long HP_CHECK_RATE   = 10000; // 10 sec
    static const unsigned long AUTO_ATTACK_RATE = 5000; // 5 sec

    void recomputeStage(); // refresh stage/baseImage from level, handle milestones
};

// XP rewards
#define XP_BLE_DEVICE 15
#define XP_BLE_ATTACK 50
#define XP_FEED       20
#define XP_PLAY       15
#define XP_AUTO_ATTACK 5

// HP/MP rewards on discovery / attack / feed
#define HP_BLE_FOUND  5
#define HP_BLE_ATTACK 10
#define MP_BLE_FOUND  8
#define MP_BLE_ATTACK 15
#define MP_FEED       15

#endif // PET_STATS_H
