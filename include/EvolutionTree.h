/**
 * @file EvolutionTree.h
 * @brief Digimon-style branching evolution model for NeonGhost.
 *
 * Design (see plan):
 *  - 5 ARCHETYPES (style lines): GENESIS, JAMMER, SPAMMER, SNIFFER, STRIKER.
 *  - Each archetype has 10 STAGES (1..10). Stages are mostly additive (auras/FX),
 *    except MILESTONES at stage 4, 8 and 10 where the base image changes AND a
 *    line-jump may happen.
 *  - The route the pet takes is driven by AFFINITIES accumulated from what it
 *    does most (BT deauth, BLE spam, sniffing, WiFi). At a milestone the pet may
 *    semi-randomly JUMP to the archetype matching its dominant affinity.
 *  - NO gating: every power is always available. Stage only grows visuals and
 *    POWER MASTERY (potency). Manual use marks preference (feeds affinity).
 *
 * This header is pure data + small inline helpers (no dynamic allocation), safe
 * to include from both the stat manager and the renderer.
 */
#ifndef EVOLUTION_TREE_H
#define EVOLUTION_TREE_H

#include <Arduino.h>

// =============================================================================
// ARCHETYPES (the 5 "looks" / style lines)
// =============================================================================
enum Archetype : uint8_t {
    ARCH_GENESIS = 0,  // neutral newborn / care line
    ARCH_JAMMER,       // BT Classic disruption ("speaker killer") line
    ARCH_SPAMMER,      // BLE spam line
    ARCH_SNIFFER,      // scan / sniff / recon line
    ARCH_STRIKER,      // WiFi deauth / recon line
    ARCHETYPE_COUNT
};

// The 4 specialization affinities (GENESIS is the unspecialized origin and has
// no affinity counter of its own).
enum Route : uint8_t {
    ROUTE_JAMMER = 0,
    ROUTE_SPAMMER,
    ROUTE_SNIFFER,
    ROUTE_STRIKER,
    ROUTE_COUNT
};

static const char* ARCHETYPE_NAMES[ARCHETYPE_COUNT] = {
    "GENESIS", "JAMMER", "SPAMMER", "SNIFFER", "STRIKER"
};

// Per-archetype tint (RGB565). Stage adds aura intensity on top of this.
static const uint16_t ARCHETYPE_COLORS[ARCHETYPE_COUNT] = {
    0x07FF, // GENESIS - cyan
    0xF800, // JAMMER  - red
    0xF81F, // SPAMMER - magenta
    0x07E0, // SNIFFER - green
    0xFFE0  // STRIKER - yellow
};

static const uint16_t ARCHETYPE_GLOW[ARCHETYPE_COUNT] = {
    0x0410, 0x6000, 0x6008, 0x0320, 0x6300
};

// SD folder name per archetype: /sprites/<folder>/stage_<base>_<frame>.bin
static const char* ARCHETYPE_FOLDER[ARCHETYPE_COUNT] = {
    "genesis", "jammer", "spammer", "sniffer", "striker"
};

// =============================================================================
// POWERS (always available, no gating). Each power has a "home" archetype it
// is most effective in; foreign powers cost more MP / hit softer.
// =============================================================================
enum PowerKind : uint8_t {
    PK_BLE,        // dispatched through BLEScanner
    PK_BTCLASSIC,  // dispatched through BTClassicManager (best-effort)
    PK_WIFI,       // dispatched through WiFiAttackManager
    PK_RECON       // passive scan / capture
};

struct PowerDef {
    const char* name;
    Archetype   home;      // archetype this power belongs to (preference)
    PowerKind   kind;
    uint8_t     baseMpCost;
    uint16_t    cooldownMs;
};

// Keep this table the single source of truth for abilities. Index == power id.
static const PowerDef POWERS[] = {
    // GENESIS — basics, cheap
    { "Ping Pulse",     ARCH_GENESIS, PK_RECON,     3,  8000 },  // 0
    { "BLE Sniff",      ARCH_GENESIS, PK_RECON,     4, 10000 },  // 1
    // JAMMER — BT Classic disruption (speaker focus)
    { "BT Inquiry",     ARCH_JAMMER,  PK_BTCLASSIC, 8, 12000 },  // 2
    { "A2DP Choke",     ARCH_JAMMER,  PK_BTCLASSIC,12, 18000 },  // 3
    { "RFCOMM Occupy",  ARCH_JAMMER,  PK_BTCLASSIC,14, 20000 },  // 4
    { "Speaker Kill",   ARCH_JAMMER,  PK_BTCLASSIC,18, 25000 },  // 5
    // SPAMMER — BLE advertising spam
    { "Sour Apple",     ARCH_SPAMMER, PK_BLE,       6, 15000 },  // 6
    { "Swift Pair",     ARCH_SPAMMER, PK_BLE,       6, 15000 },  // 7
    { "Samsung Spam",   ARCH_SPAMMER, PK_BLE,      10, 18000 },  // 8
    { "AirTag Spam",    ARCH_SPAMMER, PK_BLE,       8, 20000 },  // 9
    { "BLE Flood",      ARCH_SPAMMER, PK_BLE,      10, 18000 },  // 10
    // SNIFFER — recon / capture
    { "Device Track",   ARCH_SNIFFER, PK_RECON,     6, 14000 },  // 11
    { "PCAP Capture",   ARCH_SNIFFER, PK_RECON,     8, 16000 },  // 12
    // STRIKER — WiFi
    { "WiFi Scan",      ARCH_STRIKER, PK_WIFI,       8, 16000 }, // 13
    { "Deauth Strike",  ARCH_STRIKER, PK_WIFI,      16, 25000 }, // 14
    { "Beacon Spam",    ARCH_STRIKER, PK_WIFI,      12, 22000 }, // 15
};
#define POWER_COUNT (sizeof(POWERS) / sizeof(POWERS[0]))

// =============================================================================
// STAGE / MILESTONE HELPERS
// =============================================================================
#define MAX_STAGE 10

// Base image index (0..3) for a given stage. New base art at stages 4, 8, 10.
inline uint8_t baseImageForStage(uint8_t stage) {
    if (stage >= 10) return 3;
    if (stage >= 8)  return 2;
    if (stage >= 4)  return 1;
    return 0;
}

inline bool isMilestone(uint8_t stage) {
    return stage == 4 || stage == 8 || stage == 10;
}

// Map total pet level to a stage (1..10) inside the current line. Tunable.
// Early stages are quick, later ones slower (a long-lived pet).
inline uint8_t stageFromLevel(int level) {
    if (level <= 1)  return 1;
    if (level <= 3)  return 2;
    if (level <= 5)  return 3;
    if (level <= 8)  return 4;   // milestone
    if (level <= 11) return 5;
    if (level <= 15) return 6;
    if (level <= 19) return 7;
    if (level <= 24) return 8;   // milestone
    if (level <= 30) return 9;
    return 10;                    // APEX milestone
}

// Index of the dominant affinity (returns a Route). Ties favor JAMMER (BT focus).
inline Route dominantRoute(const uint32_t aff[ROUTE_COUNT]) {
    Route best = ROUTE_JAMMER;
    uint32_t bestVal = aff[ROUTE_JAMMER];
    for (uint8_t r = 1; r < ROUTE_COUNT; r++) {
        if (aff[r] > bestVal) { bestVal = aff[r]; best = (Route)r; }
    }
    return best;
}

// Archetype that a Route leads to.
inline Archetype archetypeForRoute(Route r) {
    switch (r) {
        case ROUTE_JAMMER:  return ARCH_JAMMER;
        case ROUTE_SPAMMER: return ARCH_SPAMMER;
        case ROUTE_SNIFFER: return ARCH_SNIFFER;
        case ROUTE_STRIKER: return ARCH_STRIKER;
        default:            return ARCH_GENESIS;
    }
}

// Route a power belongs to (for preference accounting). GENESIS powers feed no
// route; return ROUTE_COUNT to signal "no preference".
inline Route routeForArchetype(Archetype a) {
    switch (a) {
        case ARCH_JAMMER:  return ROUTE_JAMMER;
        case ARCH_SPAMMER: return ROUTE_SPAMMER;
        case ARCH_SNIFFER: return ROUTE_SNIFFER;
        case ARCH_STRIKER: return ROUTE_STRIKER;
        default:           return ROUTE_COUNT;
    }
}

/**
 * Decide whether/where to jump lines at a milestone. Semi-random: probability is
 * weighted by how dominant the strongest affinity is relative to the total.
 * Returns the new archetype (may equal current = no jump).
 *
 * @param current  current archetype
 * @param aff      affinity counters
 * @param roll     a random value 0..99 (caller supplies, e.g. random(0,100))
 */
inline Archetype evalLineJump(Archetype current, const uint32_t aff[ROUTE_COUNT], int roll) {
    uint32_t total = 0;
    for (uint8_t r = 0; r < ROUTE_COUNT; r++) total += aff[r];
    if (total < 5) return current;                 // not enough signal yet

    Route dom = dominantRoute(aff);
    Archetype target = archetypeForRoute(dom);
    if (target == current) return current;          // already on the dominant line

    // Jump chance scales with dominance share (40%..95%).
    uint32_t share = (aff[dom] * 100) / total;      // 0..100
    int chance = 40 + (int)(share / 2);             // 40 + up to 50
    if (chance > 95) chance = 95;
    return (roll < chance) ? target : current;
}

#endif // EVOLUTION_TREE_H
