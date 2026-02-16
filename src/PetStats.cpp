/**
 * @file PetStats.cpp
 * @description Advanced Statistics Manager for the NeonGhost Cyber-Pet.
 * Implements game loop logic for XP, Levels, HP/MP decay, and hierarchical evolution stages.
 * Features persistent state management via ESP32 Preferences and real-time state machine updates.
 * 
 * @author Alejandro Ramírez
 * @version 7.0 (Production Grade)
 */

#include "PetStats.h"
#include "GhostSprites.h"
#include <Preferences.h>

// Global persistent storage handle for ESP32 Non-Volatile Storage (NVS)
extern Preferences prefs;

/**
 * Initializes the statistics structure to default "Genesis" values.
 * Used during first boot or hard reset.
 */
void PetStatsManager::init() {
    stats.hp = 100;
    stats.mp = 100;
    stats.level = 1;
    stats.xp = 0;
    stats.xpMax = 100;
    stats.stage = STAGE_SPARK;
    
    unsigned long now = millis();
    stats.lastMPDecay = now;
    stats.lastHPCheck = now;
    stats.lastActivity = now;
    stats.lastAutoAttack = now;
    stats.isSleeping = false;
    stats.isDead = false;
}

/**
 * Loads the pet's state from non-volatile memory.
 * Recalculates thresholds based on current level to ensure consistency.
 */
void PetStatsManager::load() {
    // Open the "ghost" namespace in read-only mode
    prefs.begin("ghost", true);
    stats.level = prefs.getInt("lvl", 1);
    stats.xp = prefs.getInt("xp", 0);
    stats.hp = prefs.getUChar("hp", 100);
    stats.mp = prefs.getUChar("mp", 100);
    prefs.end();
    
    // Dynamic XP threshold scaling: level * 50 + 50
    stats.xpMax = stats.level * 50 + 50;
    stats.stage = (EvolutionStage)getStageFromLevel(stats.level);
    
    // Initialize timing markers for the game loop
    unsigned long now = millis();
    stats.lastMPDecay = now;
    stats.lastHPCheck = now;
    stats.lastActivity = now;
    stats.lastAutoAttack = now;
    stats.isSleeping = false;
    stats.isDead = false;
}

/**
 * Persists the current statistics to flash memory.
 * Ensures state survival across power cycles.
 */
void PetStatsManager::save() {
    prefs.begin("ghost", false);
    prefs.putInt("lvl", stats.level);
    prefs.putInt("xp", stats.xp);
    prefs.putUChar("hp", stats.hp);
    prefs.putUChar("mp", stats.mp);
    prefs.end();
}

/**
 * Resets the pet to factory settings and wipes persistent storage.
 */
void PetStatsManager::reset() {
    init();
    save();
}

/**
 * Main game logic update loop.
 * Should be called once per frame/tick.
 */
void PetStatsManager::update() {
    unsigned long now = millis();
    
    decayMP(); // Energy depletion
    checkHP();  // Health maintenance/starvation check
    
    // Auto-sleep logic: enters low-energy state after 2 minutes of inactivity
    if (!stats.isSleeping && (now - stats.lastActivity > 120000)) {
        stats.isSleeping = true;
    }
}

/**
 * Handles continuous MP (Mana/Energy) decay.
 * Rate is throttled when the pet is in sleeping mode.
 */
void PetStatsManager::decayMP() {
    unsigned long now = millis();
    
    // Multiplier for sleep mode to conserve energy
    unsigned long rate = stats.isSleeping ? MP_DECAY_RATE * 3 : MP_DECAY_RATE;
    
    if (now - stats.lastMPDecay > rate) {
        if (stats.mp > 0) stats.mp--;
        stats.lastMPDecay = now;
    }
}

// Internal counter to stagger health loss during starvation
static uint8_t hpDrainCounter = 0;

/**
 * Monitors health and handles starvation/recovery mechanics.
 */
void PetStatsManager::checkHP() {
    unsigned long now = millis();
    
    if (now - stats.lastHPCheck > HP_CHECK_RATE) {
        // Starvation: Health depletes when energy (MP) hits zero
        if (stats.mp == 0 && stats.hp > 0) {
            stats.hp--;
        }
        
        // Passive Recovery: Health regenerates if pet is well-fed (> 30 MP)
        if (stats.mp > 30 && stats.hp < 100) {
            stats.hp++;
        }
        
        // Final Death State Transition
        if (stats.hp == 0 && !stats.isDead) {
            stats.isDead = true;
            save(); // Lock death state in storage
        }
        
        stats.lastHPCheck = now;
    }
}

/**
 * Re-animates a dead pet with a significant XP penalty.
 */
void PetStatsManager::revive() {
    if (!stats.isDead) return;
    
    // Penalty calculation: 10% loss or flat 50 XP (whichever is higher)
    int xpLoss = stats.xp / 10;
    if (xpLoss < 50) xpLoss = 50;
    
    stats.xp = max(0, stats.xp - xpLoss);
    
    // Partial restoration of vitals
    stats.hp = 50;
    stats.mp = 50;
    stats.isDead = false;
    stats.lastActivity = millis();
    
    save();
}

/**
 * Replenishes energy and health. Triggers a wake-up event if pet is sleeping.
 */
void PetStatsManager::feed(int amount) {
    addMP(amount);
    addHP(amount / 4); // Secondary healing effect from food
    stats.lastActivity = millis();
    if (stats.isSleeping) wake();
}

void PetStatsManager::rest() {
    stats.isSleeping = true;
}

void PetStatsManager::wake() {
    stats.isSleeping = false;
    stats.lastActivity = millis();
}

/**
 * Orchestrates autonomous attack behavior for background interaction.
 * @returns {bool} True if an attack was successfully launched.
 */
bool PetStatsManager::tryAutoAttack() {
    unsigned long now = millis();
    
    if (stats.isSleeping) return false;
    if (now - stats.lastAutoAttack < AUTO_ATTACK_RATE) return false;
    
    int cost = getAttackMPCost();
    if (stats.mp < cost) return false;
    
    stats.mp -= cost;
    stats.lastAutoAttack = now;
    stats.lastActivity = now;
    
    return true;
}

/**
 * Dynamic calculation of attack costs based on current evolution stage.
 * Scaling: 5 + (Stage * 2)
 */
int PetStatsManager::getAttackMPCost() {
    return 5 + (stats.stage * 2);
}

/**
 * Core progression logic. Handles XP accumulation and multi-level overflows.
 */
void PetStatsManager::addXP(int amount) {
    stats.xp += amount;
    stats.lastActivity = millis();
    
    if (stats.isSleeping) wake();
    
    // Process multiple level-ups if XP gain is massive
    while (stats.xp >= stats.xpMax) {
        stats.xp -= stats.xpMax;
        stats.level++;
        stats.xpMax = stats.level * 50 + 50; // Re-scale threshold
        evolve(); // Check for stage transition
    }
}

void PetStatsManager::addHP(int amount) {
    stats.hp = min(100, stats.hp + amount);
}

void PetStatsManager::addMP(int amount) {
    stats.mp = min(100, stats.mp + amount);
}

bool PetStatsManager::checkLevelUp() {
    return stats.xp >= stats.xpMax;
}

/**
 * Checks for evolution stage transitions and handles the visual/stat updates.
 */
void PetStatsManager::evolve() {
    EvolutionStage oldStage = stats.stage;
    stats.stage = (EvolutionStage)getStageFromLevel(stats.level);
    
    if (stats.stage != oldStage) {
        // Adaptive reward: Full restoration upon reaching a new evolution stage
        stats.hp = 100;
        stats.mp = 100;
    }
    save();
}

EvolutionStage PetStatsManager::getStage() {
    return stats.stage;
}

/**
 * Mapping of numerical level to evolution stage archetype.
 * Defines the progression hierarchy from SPARK to DAEMON.
 */
int PetStatsManager::getStageFromLevel(int level) {
    if (level <= 2) return 0;   // SPARK
    if (level <= 4) return 1;   // BYTE
    if (level <= 6) return 2;   // GHOST
    if (level <= 9) return 3;   // SPECTER
    if (level <= 12) return 4;  // PHANTOM
    if (level <= 15) return 5;  // WRAITH
    if (level <= 18) return 6;  // SHADE
    if (level <= 22) return 7;  // REVENANT
    if (level <= 26) return 8;  // BANSHEE
    if (level <= 30) return 9;  // LICH
    if (level <= 35) return 10; // POLTERGEIST
    if (level <= 40) return 11; // VOID
    if (level <= 47) return 12; // NIGHTMARE
    if (level <= 55) return 13; // REAPER
    return 14;                  // DAEMON
}

const char* PetStatsManager::getStageName() {
    return STAGE_NAMES[stats.stage];
}

const char* PetStatsManager::getAttackName() {
    return ATTACK_NAMES[stats.stage];
}

uint8_t PetStatsManager::getAttackType() {
    return ATTACK_TYPES[stats.stage];
}

/**
 * Returns the UI theme color associated with the current evolution stage.
 */
uint16_t PetStatsManager::getStageColor() {
    return STAGE_COLORS[stats.stage];
}
