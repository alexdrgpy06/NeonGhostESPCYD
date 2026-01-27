/**
 * ╔═══════════════════════════════════════════════════════════════╗
 * ║              PET STATISTICS MANAGER v7.0                      ║
 * ║   Handles XP/Level/HP/MP logic, saving, and evolution         ║
 * ║                                                               ║
 * ║   Features: Death/Revive System, Auto-Save, Stage Colors      ║
 * ║                                                               ║
 * ║                     by Alex R.                                ║
 * ╚═══════════════════════════════════════════════════════════════╝
 */
#include "PetStats.h"
#include "GhostSprites.h"
#include <Preferences.h>

extern Preferences prefs;

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

void PetStatsManager::load() {
    prefs.begin("ghost", true);
    stats.level = prefs.getInt("lvl", 1);
    stats.xp = prefs.getInt("xp", 0);
    stats.hp = prefs.getUChar("hp", 100);
    stats.mp = prefs.getUChar("mp", 100);
    prefs.end();
    
    stats.xpMax = stats.level * 50 + 50;
    stats.stage = (EvolutionStage)getStageFromLevel(stats.level);
    
    unsigned long now = millis();
    stats.lastMPDecay = now;
    stats.lastHPCheck = now;
    stats.lastActivity = now;
    stats.lastAutoAttack = now;
    stats.isSleeping = false;
    stats.isDead = false;
}

void PetStatsManager::save() {
    prefs.begin("ghost", false);
    prefs.putInt("lvl", stats.level);
    prefs.putInt("xp", stats.xp);
    prefs.putUChar("hp", stats.hp);
    prefs.putUChar("mp", stats.mp);
    prefs.end();
}

void PetStatsManager::reset() {
    init();
    save();
}

void PetStatsManager::update() {
    unsigned long now = millis();
    
    decayMP();
    checkHP();
    
    // Auto-sleep after 2 minutes
    if (!stats.isSleeping && (now - stats.lastActivity > 120000)) {
        stats.isSleeping = true;
    }
}

void PetStatsManager::decayMP() {
    unsigned long now = millis();
    
    // MP decays slower when sleeping
    unsigned long rate = stats.isSleeping ? MP_DECAY_RATE * 3 : MP_DECAY_RATE;
    
    if (now - stats.lastMPDecay > rate) {
        if (stats.mp > 0) stats.mp--;
        stats.lastMPDecay = now;
    }
}

// Counter for slower HP drain when MP is empty
static uint8_t hpDrainCounter = 0;

void PetStatsManager::checkHP() {
    unsigned long now = millis();
    
    if (now - stats.lastHPCheck > HP_CHECK_RATE) {
        // HP drops if MP is 0 (but slower - every 3rd tick)
        // HP drops if MP is 0 (Starvation)
        if (stats.mp == 0 && stats.hp > 0) {
            stats.hp--;
        }
        
        // HP recovers when MP > 30 (was 50 - more forgiving)
        if (stats.mp > 30 && stats.hp < 100) {
            stats.hp++;
        }
        
        // DEATH MECHANIC: HP=0 causes XP loss
        // DEATH MECHANIC
        if (stats.hp == 0 && !stats.isDead) {
            stats.isDead = true;
            save(); // Save death state
        }
        
        stats.lastHPCheck = now;
    }
}

void PetStatsManager::revive() {
    if (!stats.isDead) return;
    
    // XP Penalty (10% or min 50)
    int xpLoss = stats.xp / 10;
    if (xpLoss < 50) xpLoss = 50;
    
    stats.xp = max(0, stats.xp - xpLoss);
    
    // Restore partial stats
    stats.hp = 50;
    stats.mp = 50;
    stats.isDead = false;
    stats.lastActivity = millis();
    
    save();
}


void PetStatsManager::feed(int amount) {
    addMP(amount);
    addHP(amount / 4);
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

int PetStatsManager::getAttackMPCost() {
    // Higher stages = more powerful but costlier attacks
    return 5 + (stats.stage * 2);
}

void PetStatsManager::addXP(int amount) {
    stats.xp += amount;
    stats.lastActivity = millis();
    
    if (stats.isSleeping) wake();
    
    while (stats.xp >= stats.xpMax) {
        stats.xp -= stats.xpMax;
        stats.level++;
        stats.xpMax = stats.level * 50 + 50;
        evolve();
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

void PetStatsManager::evolve() {
    EvolutionStage oldStage = stats.stage;
    stats.stage = (EvolutionStage)getStageFromLevel(stats.level);
    
    if (stats.stage != oldStage) {
        // Full restore on evolution
        stats.hp = 100;
        stats.mp = 100;
    }
    save();
}

EvolutionStage PetStatsManager::getStage() {
    return stats.stage;
}

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

uint16_t PetStatsManager::getStageColor() {
    return STAGE_COLORS[stats.stage];
}
