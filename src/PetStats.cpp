/**
 * @file PetStats.cpp
 * @brief Branching evolution + NG+ stat manager for the NeonGhost cyber-pet.
 *
 * Replaces the old linear 15-stage system with a Digimon-style model:
 *   5 archetypes x 10 stages, affinity-driven line jumps at milestones (4/8/10),
 *   persistent power mastery and lifetime counters (New Game Plus).
 *
 * @author Alejandro Ramírez
 */

#include "PetStats.h"
#include <Preferences.h>

extern Preferences prefs;

// -----------------------------------------------------------------------------
// Init / persistence
// -----------------------------------------------------------------------------
void PetStatsManager::init() {
    stats.hp = 100;
    stats.mp = 100;
    stats.level = 1;
    stats.xp = 0;
    stats.xpMax = 100;
    stats.archetype = ARCH_GENESIS;
    stats.stage = 1;
    stats.baseImage = 0;

    for (uint8_t r = 0; r < ROUTE_COUNT; r++) stats.affinity[r] = 0;
    for (uint8_t p = 0; p < POWER_COUNT; p++) stats.powerMastery[p] = 0;

    stats.totalAttacks = 0;
    stats.devicesFound = 0;
    stats.lineJumps = 0;
    stats.powerupsUnlocked = 0;
    stats.deaths = 0;
    stats.revives = 0;
    stats.ageBoots = 0;

    unsigned long now = millis();
    stats.lastMPDecay = now;
    stats.lastHPCheck = now;
    stats.lastActivity = now;
    stats.lastAutoAttack = now;
    stats.isSleeping = false;
    stats.isDead = false;
}

void PetStatsManager::load() {
    init(); // defaults first (also covers migration from old v8 saves)

    prefs.begin("ghost", true);
    stats.level = prefs.getInt("lvl", 1);
    stats.xp    = prefs.getInt("xp", 0);
    stats.hp    = prefs.getUChar("hp", 100);
    stats.mp    = prefs.getUChar("mp", 100);

    // New-model fields (absent on legacy saves -> defaults from init()).
    stats.archetype = (Archetype)prefs.getUChar("arch", ARCH_GENESIS);
    stats.stage     = prefs.getUChar("stg", 0); // 0 => derive from level below

    prefs.getBytes("aff",  stats.affinity,     sizeof(stats.affinity));
    prefs.getBytes("mast", stats.powerMastery, sizeof(stats.powerMastery));

    stats.totalAttacks     = prefs.getUInt("tA", 0);
    stats.devicesFound     = prefs.getUInt("dF", 0);
    stats.lineJumps        = prefs.getUInt("lJ", 0);
    stats.powerupsUnlocked = prefs.getUInt("pU", 0);
    stats.deaths           = prefs.getUShort("dth", 0);
    stats.revives          = prefs.getUShort("rvv", 0);
    stats.ageBoots         = prefs.getUShort("age", 0);
    prefs.end();

    stats.xpMax = stats.level * 50 + 50;
    if (stats.stage < 1) stats.stage = stageFromLevel(stats.level); // migration
    if (stats.stage > MAX_STAGE) stats.stage = MAX_STAGE;
    stats.baseImage = baseImageForStage(stats.stage);

    // Count this boot.
    stats.ageBoots++;

    unsigned long now = millis();
    stats.lastMPDecay = now;
    stats.lastHPCheck = now;
    stats.lastActivity = now;
    stats.lastAutoAttack = now;
    stats.isSleeping = false;
    stats.isDead = (stats.hp == 0);
}

void PetStatsManager::save() {
    prefs.begin("ghost", false);
    prefs.putInt("lvl", stats.level);
    prefs.putInt("xp", stats.xp);
    prefs.putUChar("hp", stats.hp);
    prefs.putUChar("mp", stats.mp);
    prefs.putUChar("arch", (uint8_t)stats.archetype);
    prefs.putUChar("stg", stats.stage);
    prefs.putBytes("aff",  stats.affinity,     sizeof(stats.affinity));
    prefs.putBytes("mast", stats.powerMastery, sizeof(stats.powerMastery));
    prefs.putUInt("tA", stats.totalAttacks);
    prefs.putUInt("dF", stats.devicesFound);
    prefs.putUInt("lJ", stats.lineJumps);
    prefs.putUInt("pU", stats.powerupsUnlocked);
    prefs.putUShort("dth", stats.deaths);
    prefs.putUShort("rvv", stats.revives);
    prefs.putUShort("age", stats.ageBoots);
    prefs.end();
}

void PetStatsManager::reset() {
    prefs.begin("ghost", false);
    prefs.clear();
    prefs.end();
    init();
    save();
}

// -----------------------------------------------------------------------------
// Game loop
// -----------------------------------------------------------------------------
void PetStatsManager::update() {
    unsigned long now = millis();
    decayMP();
    checkHP();
    if (!stats.isSleeping && (now - stats.lastActivity > 120000)) {
        stats.isSleeping = true;
    }
}

void PetStatsManager::decayMP() {
    unsigned long now = millis();
    unsigned long rate = stats.isSleeping ? MP_DECAY_RATE * 3 : MP_DECAY_RATE;
    if (now - stats.lastMPDecay > rate) {
        if (stats.mp > 0) stats.mp--;
        stats.lastMPDecay = now;
    }
}

void PetStatsManager::checkHP() {
    unsigned long now = millis();
    if (now - stats.lastHPCheck > HP_CHECK_RATE) {
        if (stats.mp == 0 && stats.hp > 0) stats.hp--;
        if (stats.mp > 30 && stats.hp < 100) stats.hp++;
        if (stats.hp == 0 && !stats.isDead) {
            stats.isDead = true;
            stats.deaths++;
            save();
        }
        stats.lastHPCheck = now;
    }
}

void PetStatsManager::revive() {
    if (!stats.isDead) return;
    int xpLoss = stats.xp / 10;
    if (xpLoss < 50) xpLoss = 50;
    stats.xp = max(0, stats.xp - xpLoss);
    stats.hp = 50;
    stats.mp = 50;
    stats.isDead = false;
    stats.revives++;
    stats.lastActivity = millis();
    save();
}

void PetStatsManager::feed(int amount) {
    addMP(amount);
    addHP(amount / 4);
    stats.lastActivity = millis();
    if (stats.isSleeping) wake();
}

void PetStatsManager::rest() { stats.isSleeping = true; }

void PetStatsManager::wake() {
    stats.isSleeping = false;
    stats.lastActivity = millis();
}

// -----------------------------------------------------------------------------
// Auto-attack pacing
// -----------------------------------------------------------------------------
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
    return 5 + (stats.stage * 1);
}

// -----------------------------------------------------------------------------
// Progression
// -----------------------------------------------------------------------------
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

void PetStatsManager::addHP(int amount) { stats.hp = min(100, stats.hp + amount); }
void PetStatsManager::addMP(int amount) { stats.mp = min(100, stats.mp + amount); }

bool PetStatsManager::checkLevelUp() { return stats.xp >= stats.xpMax; }

void PetStatsManager::recomputeStage() {
    uint8_t s = stageFromLevel(stats.level);
    if (s > MAX_STAGE) s = MAX_STAGE;
    stats.stage = s;
    stats.baseImage = baseImageForStage(s);
}

/**
 * Stage progression + milestone handling (visual base change, potency bump,
 * and possible semi-random line jump weighted by affinity).
 */
void PetStatsManager::evolve() {
    uint8_t newStage = stageFromLevel(stats.level);
    if (newStage > MAX_STAGE) newStage = MAX_STAGE;
    if (newStage <= stats.stage) { save(); return; }

    bool hitMilestone = false;
    for (uint8_t s = stats.stage + 1; s <= newStage; s++) {
        if (isMilestone(s)) { hitMilestone = true; stats.powerupsUnlocked++; }
    }

    stats.stage = newStage;
    stats.baseImage = baseImageForStage(newStage);
    justEvolved = true;

    if (hitMilestone) {
        stats.hp = 100;
        stats.mp = 100;
        Archetype nt = evalLineJump(stats.archetype, stats.affinity, random(0, 100));
        if (nt != stats.archetype) {
            stats.archetype = nt;
            stats.lineJumps++;
            justJumped = true;
        }
    }
    save();
}

// -----------------------------------------------------------------------------
// Branching / NG+
// -----------------------------------------------------------------------------
void PetStatsManager::addAffinity(Route r, uint32_t n) {
    if (r >= ROUTE_COUNT) return;
    stats.affinity[r] += n;
}

void PetStatsManager::gainMastery(uint8_t powerId, uint16_t n) {
    if (powerId >= POWER_COUNT) return;
    uint32_t m = (uint32_t)stats.powerMastery[powerId] + n;
    if (m > 9999) m = 9999;
    stats.powerMastery[powerId] = (uint16_t)m;
    stats.totalAttacks++;
}

uint16_t PetStatsManager::masteryOf(uint8_t powerId) {
    if (powerId >= POWER_COUNT) return 0;
    return stats.powerMastery[powerId];
}

Route PetStatsManager::dominant() { return dominantRoute(stats.affinity); }

int PetStatsManager::powerMpCost(uint8_t powerId) {
    if (powerId >= POWER_COUNT) return 99;
    int c = POWERS[powerId].baseMpCost;
    Archetype home = POWERS[powerId].home;
    // Foreign (non-home, non-genesis) powers cost ~33% more.
    if (home != stats.archetype && home != ARCH_GENESIS) c += c / 3;
    // Mastery shaves up to ~40% off.
    uint16_t m = stats.powerMastery[powerId];
    int reduce = (m > 200) ? (c * 40 / 100) : (c * (m / 5) / 100);
    c -= reduce;
    if (c < 1) c = 1;
    return c;
}

// -----------------------------------------------------------------------------
// Getters
// -----------------------------------------------------------------------------
Archetype   PetStatsManager::getArchetype()     { return stats.archetype; }
uint8_t     PetStatsManager::getStage()         { return stats.stage; }
const char* PetStatsManager::getArchetypeName() { return ARCHETYPE_NAMES[stats.archetype]; }
uint16_t    PetStatsManager::getArchetypeColor(){ return ARCHETYPE_COLORS[stats.archetype]; }
uint16_t    PetStatsManager::getArchetypeGlow() { return ARCHETYPE_GLOW[stats.archetype]; }
