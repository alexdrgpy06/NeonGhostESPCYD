/**
 * ╔═══════════════════════════════════════════════════════════════╗
 * ║                    NEONGHOST v7.0                             ║
 * ║              Hacker Pet for ESP32-CYD                         ║
 * ║                                                               ║
 * ║   Features: WiFi Sniffing, BLE Scanning, RGB LED Sync         ║
 * ║   15 Evolution Stages, Death/Revive System                    ║
 * ║                                                               ║
 * ║                     by Alex R.                                ║
 * ╚═══════════════════════════════════════════════════════════════╝
 */
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>

#include "CreatureRenderer.h"
#include "PacketSniffer.h"
#include "SDManager.h"
#include "BLEScanner.h"
#include "GhostSprites.h"
#include "PetStats.h"

// =============================================================================
// HARDWARE
// =============================================================================
#define CYD_TFT_BL     21
#define CYD_TOUCH_CS   33
#define CYD_TOUCH_IRQ  36
#define CYD_TOUCH_MOSI 32
#define CYD_TOUCH_MISO 39
#define CYD_TOUCH_CLK  25
#define CYD_LED_RED    4
#define CYD_LED_GREEN  16
#define CYD_LED_BLUE   17
void updatePhysicalLED(uint16_t color); // Forward declaration

// Colors
#define C_BG       0x0000
#define C_PANEL    0x18C3
#define C_GREEN    0x07E0
#define C_CYAN     0x07FF
#define C_ORANGE   0xFD20
#define C_RED      0xF800
#define C_BLUE     0x001F
#define C_WHITE    0xFFFF
#define C_GREY     0x4208
#define C_DARK     0x2104

// =============================================================================
// OBJECTS
// =============================================================================
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(CYD_TOUCH_CS, CYD_TOUCH_IRQ);
Preferences prefs;
CreatureRenderer creature(&tft);
PacketSniffer wifiSniffer;
SDManager sdManager;
BLEScanner bleScanner;
PetStatsManager petStats;

// =============================================================================
// LAYOUT
// =============================================================================
#define TOP_BAR_H      45
#define GHOST_AREA_Y   50
#define GHOST_AREA_H   165
#define STATUS_BAR_Y   220
#define STATUS_BAR_H   45
#define BUTTON_Y       270
#define BUTTON_H       45
#define BUTTON_W       70

// =============================================================================
// STATE
// =============================================================================
struct GameState {
    String statusLine1 = "";
    String statusLine2 = "";
    uint16_t themeColor = C_GREEN;
    unsigned long alertStart = 0;
    unsigned long lastSave = 0;
    unsigned long lastBleScan = 0;
    unsigned long lastAbilityRun = 0;
    
    bool inListView = false;
    bool inMenuView = false;
    bool showingWifi = true;
    int listScroll = 0;
    int menuScroll = 0;
    
    // Ghost position (free movement)
    float ghostX = 120;
    float ghostY = 130;
    float ghostVelX = 0.5;
    float ghostBob = 0;
    
    // Dirty flags
    bool topBarDirty = true;
    bool statusDirty = true;
    bool ghostDirty = true;
    
    // Reset combo (5 taps on top bar)
    int resetTapCount = 0;
    unsigned long lastResetTap = 0;
    
    // Previous values
    int lastLevel = -1;
    int lastXP = -1;
    int lastHP = -1;
    int lastMP = -1;
} game;

void setStatus(String line1, String line2, uint16_t color);
void drawButtons();
void drawTopBar();
void drawStatusBar();
void drawGhostArea();
void fullReset();

// =============================================================================
// CUMULATIVE ABILITIES (all stages up to current run)
// =============================================================================
struct Ability {
    const char* name;
    uint8_t type;  // 0=WiFi, 1=BT
    int mpCost;
    unsigned long cooldown;
    unsigned long lastRun;
};

Ability abilities[] = {
    {"Probe Sniff", 0, 3, 8000, 0},     // SPARK
    {"BLE Sniff", 1, 3, 10000, 0},      // BYTE
    {"AP Spam", 0, 5, 12000, 0},        // GHOST
    {"Device Track", 1, 5, 15000, 0},   // SPECTER
    {"Deauth Attack", 0, 10, 18000, 0}, // PHANTOM (was Detect)
    {"Swift Pair", 1, 8, 20000, 0},     // WRAITH
    {"Handshake Cap", 0, 10, 25000, 0}, // SHADE
    {"Sour Apple", 1, 10, 25000, 0},    // REVENANT
    {"Rick Roll", 0, 12, 30000, 0},     // BANSHEE (was PMKID Grab)
    {"AirTag Spam", 1, 12, 30000, 0},   // LICH
    {"BLE Flood", 1, 15, 35000, 0},     // POLTERGEIST
    {"Samsung Spam", 1, 15, 35000, 0},  // VOID (was Beacon Spam)
    {"Full Scan", 0, 18, 40000, 0},     // NIGHTMARE
    {"Crash Attack", 1, 20, 45000, 0},  // REAPER
    {"Arsenal", 2, 25, 50000, 0}        // DAEMON
};
#define ABILITY_COUNT 15

// Run random ability from available pool
void processAbilities() {
    if (creature.isAnimating()) return; // Don't interrupt

    unsigned long now = millis();
    int stage = petStats.stats.stage;
    
    // Find all ready abilities
    int available[ABILITY_COUNT];
    int count = 0;
    
    for (int i = 0; i <= stage && i < ABILITY_COUNT; i++) {
        if (now - abilities[i].lastRun > abilities[i].cooldown) {
            if (petStats.stats.mp >= abilities[i].mpCost) {
                available[count++] = i;
            }
        }
    }
    
    // Pick one randomly
    if (count > 0) {
        int idx = available[random(0, count)];
        Ability* a = &abilities[idx];
        
        petStats.stats.mp -= a->mpCost;
        a->lastRun = now;
        
        setStatus(a->name, a->type == 0 ? "WiFi" : "BT", 
                 a->type == 0 ? C_GREEN : C_BLUE);
        
        petStats.addXP(3 + idx);
        game.statusDirty = true;
        
        // WiFi Attack
        if (a->type == 0) {
            creature.triggerAnimation(ANIM_ATTACK, 1000);
            creature.setLedFx(CreatureRenderer::LED_STROBE, C_GREEN, 1000);
            
            if (strcmp(a->name, "AP Spam") == 0) wifiSniffer.beaconSpam();
            else if (strcmp(a->name, "Rick Roll") == 0) wifiSniffer.rickRoll();
            else if (strcmp(a->name, "Deauth Attack") == 0) wifiSniffer.deauthAttack();
            
        } 
        // BLE Attack
        else {
            creature.triggerAnimation(ANIM_SCANNING, 1000);
            creature.setLedFx(CreatureRenderer::LED_PULSE, C_BLUE, 1000);
            
            if (strcmp(a->name, "Sour Apple") == 0) bleScanner.sourApple();
            else if (strcmp(a->name, "Swift Pair") == 0) bleScanner.swiftPair();
            else if (strcmp(a->name, "AirTag Spam") == 0) bleScanner.airTagSpam();
            else if (strcmp(a->name, "BLE Flood") == 0) bleScanner.bleFlood();
            else if (strcmp(a->name, "Samsung Spam") == 0) bleScanner.samsungSpam();
            else if (strcmp(a->name, "Crash Attack") == 0) bleScanner.bleFlood();
        }
        
        // Special FX for high level
        if (idx > 10) {
             creature.setLedFx(CreatureRenderer::LED_RAINBOW, 0, 1500);
        }
    }
}

// =============================================================================
// DRAWING (No Flicker)
// =============================================================================
void drawBackground() {
    tft.fillScreen(C_BG);
    
    // Top bar
    tft.fillRect(0, 0, 240, TOP_BAR_H, C_PANEL);
    tft.drawFastHLine(0, TOP_BAR_H, 240, petStats.getStageColor());
    
    // Ghost area - clean black, no grid
    tft.fillRect(0, GHOST_AREA_Y, 240, GHOST_AREA_H, C_BG);
    
    // Status bar
    tft.fillRect(0, STATUS_BAR_Y, 240, STATUS_BAR_H, C_PANEL);
    tft.drawFastHLine(0, STATUS_BAR_Y, 240, petStats.getStageColor());
    
    // Button area
    tft.fillRect(0, STATUS_BAR_Y + STATUS_BAR_H, 240, 320 - STATUS_BAR_Y - STATUS_BAR_H, C_PANEL);
    
    drawButtons();
    game.topBarDirty = true;
    game.statusDirty = true;
    game.ghostDirty = true;
}

void drawButtons() {
    uint16_t btnColor = petStats.getStageColor();
    int x1 = 15, x2 = 85, x3 = 155;
    
    tft.fillRoundRect(x1, BUTTON_Y, BUTTON_W, BUTTON_H, 6, C_DARK);
    tft.drawRoundRect(x1, BUTTON_Y, BUTTON_W, BUTTON_H, 6, btnColor);
    tft.setTextColor(btnColor, C_DARK);
    tft.setTextSize(1);
    tft.setCursor(x1 + 20, BUTTON_Y + 18);
    tft.print("FEED");
    
    tft.fillRoundRect(x2, BUTTON_Y, BUTTON_W, BUTTON_H, 6, C_DARK);
    tft.drawRoundRect(x2, BUTTON_Y, BUTTON_W, BUTTON_H, 6, btnColor);
    tft.setCursor(x2 + 13, BUTTON_Y + 18);
    tft.print("ATTACK");
    
    tft.fillRoundRect(x3, BUTTON_Y, BUTTON_W, BUTTON_H, 6, C_DARK);
    tft.drawRoundRect(x3, BUTTON_Y, BUTTON_W, BUTTON_H, 6, btnColor);
    tft.setCursor(x3 + 18, BUTTON_Y + 18);
    tft.print("MENU");
}

void drawTopBar() {
    if (!game.topBarDirty) return;
    
    bool changed = (petStats.stats.level != game.lastLevel) || 
                   (petStats.stats.xp != game.lastXP) ||
                   (petStats.stats.hp != game.lastHP) ||
                   (petStats.stats.mp != game.lastMP);
    if (!changed && !game.topBarDirty) return;
    
    tft.fillRect(0, 0, 240, TOP_BAR_H, C_PANEL);
    uint16_t col = petStats.getStageColor();
    
    // Level box - compact with Lv and number on same line
    tft.fillRoundRect(4, 4, 42, 37, 4, C_DARK);
    tft.drawRoundRect(4, 4, 42, 37, 4, col);
    tft.setTextColor(col, C_DARK);
    tft.setTextSize(1);
    tft.setCursor(8, 8);
    tft.print("Lv.");
    tft.setTextSize(2);
    // Center the level number
    int lvX = petStats.stats.level < 10 ? 15 : 10;
    tft.setCursor(lvX, 22);
    tft.print(petStats.stats.level);
    
    // Stage name - below level box
    tft.setTextSize(1);
    tft.setTextColor(col, C_PANEL);
    tft.setCursor(50, 32);
    tft.print(petStats.getStageName());
    
    // XP bar - next to level
    int xpX = 50, xpY = 5, xpW = 105, xpH = 12;
    tft.fillRect(xpX, xpY, xpW, xpH, C_DARK);
    tft.drawRect(xpX, xpY, xpW, xpH, col);
    int xpFill = map(petStats.stats.xp, 0, petStats.stats.xpMax, 0, xpW - 4);
    if (xpFill > 0) tft.fillRect(xpX + 2, xpY + 2, xpFill, xpH - 4, col);
    
    // XP numbers below bar
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(xpX, xpY + 14);
    tft.print("XP ");
    tft.print(petStats.stats.xp);
    tft.print("/");
    tft.print(petStats.stats.xpMax);
    
    // HP bar - right side
    int hpX = 160, hpY = 4;
    tft.setTextColor(C_RED, C_PANEL);
    tft.setTextSize(1);
    tft.setCursor(hpX, hpY);
    tft.print("HP");
    tft.fillRect(hpX + 15, hpY, 60, 10, C_DARK);
    tft.drawRect(hpX + 15, hpY, 60, 10, C_RED);
    int hpFill = map(petStats.stats.hp, 0, 100, 0, 56);
    if (hpFill > 0) tft.fillRect(hpX + 17, hpY + 2, hpFill, 6, C_RED);
    
    // MP bar - below HP
    int mpX = 160, mpY = 18;
    tft.setTextColor(C_CYAN, C_PANEL);
    tft.setCursor(mpX, mpY);
    tft.print("MP");
    tft.fillRect(mpX + 15, mpY, 60, 10, C_DARK);
    tft.drawRect(mpX + 15, mpY, 60, 10, C_CYAN);
    int mpFill = map(petStats.stats.mp, 0, 100, 0, 56);
    if (mpFill > 0) tft.fillRect(mpX + 17, mpY + 2, mpFill, 6, C_CYAN);
    
    // Bottom line
    tft.drawFastHLine(0, TOP_BAR_H - 1, 240, col);
    
    game.lastLevel = petStats.stats.level;
    game.lastXP = petStats.stats.xp;
    game.lastHP = petStats.stats.hp;
    game.lastMP = petStats.stats.mp;
    game.topBarDirty = false;
}

void drawStatusBar() {
    if (!game.statusDirty) return;
    
    tft.fillRect(0, STATUS_BAR_Y + 1, 240, STATUS_BAR_H - 2, C_PANEL);
    tft.setTextSize(1);
    
    tft.setTextColor(game.themeColor, C_PANEL);
    tft.setCursor(10, STATUS_BAR_Y + 8);
    tft.print(game.statusLine1);
    
    tft.setTextColor(C_WHITE, C_PANEL);
    tft.setCursor(10, STATUS_BAR_Y + 20);
    tft.print(game.statusLine2);
    
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(200, STATUS_BAR_Y + 8);
    tft.print("CH");
    tft.print(wifiSniffer.currentChannel);
    
    int y2 = STATUS_BAR_Y + 32;
    tft.setTextColor(C_GREEN, C_PANEL);
    tft.setCursor(10, y2);
    tft.print("W:");
    tft.print(wifiSniffer.getNetworkCount());
    
    tft.setTextColor(C_BLUE, C_PANEL);
    tft.setCursor(55, y2);
    tft.print("B:");
    tft.print(bleScanner.totalDevices);
    
    tft.setTextColor(wifiSniffer.handshakeCount > 0 ? C_ORANGE : C_GREY, C_PANEL);
    tft.setCursor(100, y2);
    tft.print("HS:");
    tft.print(wifiSniffer.handshakeCount);
    
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(145, y2);
    tft.print("PKT:");
    tft.print(wifiSniffer.packetCount / 1000);
    tft.print("K");
    
    game.statusDirty = false;
}

void setStatus(String line1, String line2, uint16_t color) {
    game.statusLine1 = line1;
    game.statusLine2 = line2;
    game.themeColor = color;
    game.alertStart = millis();
    game.statusDirty = true;
    petStats.stats.lastActivity = millis();
    if (petStats.stats.isSleeping) petStats.wake();
}

// =============================================================================
// GHOST MOVEMENT & ANIMATION
// =============================================================================
void updateGhostPosition() {
    // Side to side movement
    game.ghostX += game.ghostVelX;
    
    // Bounce at edges (ghost is 96px, so center can go from 48 to 192)
    if (game.ghostX < 64 || game.ghostX > 176) {
        game.ghostVelX = -game.ghostVelX;
    }
}

void drawGhost() {
    int centerX = (int)game.ghostX;
    int centerY = GHOST_AREA_Y + (GHOST_AREA_H / 2);
    
    // Track previous position for clearing
    static int prevSpriteX = -1;
    static int prevSpriteY = -1;
    
    // Calculate where sprite WILL be drawn
    int newSpriteX = centerX - 64;
    int newSpriteY = centerY - 64;
    
    // Clamp new position
    if (newSpriteX < 0) newSpriteX = 0;
    if (newSpriteX > 112) newSpriteX = 112;  // 240-128
    
    // Clear old position if it's different from new (prevents trails)
    if (prevSpriteX >= 0 && (prevSpriteX != newSpriteX || prevSpriteY != newSpriteY)) {
        // Only clear the non-overlapping parts for efficiency
        if (prevSpriteX < newSpriteX) {
            // Moved right - clear left edge
            tft.fillRect(prevSpriteX, prevSpriteY, newSpriteX - prevSpriteX, 128, C_BG);
        } else if (prevSpriteX > newSpriteX) {
            // Moved left - clear right edge  
            tft.fillRect(newSpriteX + 128, prevSpriteY, prevSpriteX - newSpriteX, 128, C_BG);
        }
    }
    
    // Draw creature
    creature.draw(centerX, centerY, petStats.stats.level, CreatureRenderer::SPARK);
    
    // Update previous position
    prevSpriteX = newSpriteX;
    prevSpriteY = newSpriteY;
    
    // Sync Physical LED with Ghost Color
    updatePhysicalLED(creature.currentColor);
    
    game.ghostDirty = false;
}

// =============================================================================
// RESET
// =============================================================================
void fullReset() {
    // Reset pet stats
    petStats.reset();
    
    // Reset WiFi data
    wifiSniffer.handshakeCount = 0;
    wifiSniffer.packetCount = 0;
    
    // Reset BLE (clear new device flag)
    bleScanner.clearNewDevice();
    
    // Reset abilities
    for (int i = 0; i < ABILITY_COUNT; i++) {
        abilities[i].lastRun = 0;
    }
    
    // Save cleared state
    petStats.save();
    
    setStatus("FULL RESET!", "All data cleared", C_RED);
    creature.triggerAnimation(ANIM_HAPPY, 1500);
    
    drawBackground();
}

// =============================================================================
// LIST VIEWS
// =============================================================================
void drawListHeader(bool isWifi) {
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 240, 35, C_PANEL);
    
    if (isWifi) {
        tft.fillRect(5, 5, 110, 25, C_GREEN);
        tft.setTextColor(C_BG, C_GREEN);
    } else {
        tft.drawRect(5, 5, 110, 25, C_GREEN);
        tft.setTextColor(C_GREEN, C_PANEL);
    }
    tft.setTextSize(1);
    tft.setCursor(20, 12);
    tft.print("WIFI (");
    tft.print(wifiSniffer.getNetworkCount());
    tft.print(")");
    
    if (!isWifi) {
        tft.fillRect(125, 5, 110, 25, C_BLUE);
        tft.setTextColor(C_WHITE, C_BLUE);
    } else {
        tft.drawRect(125, 5, 110, 25, C_BLUE);
        tft.setTextColor(C_BLUE, C_PANEL);
    }
    tft.setCursor(145, 12);
    tft.print("BLE (");
    tft.print(bleScanner.totalDevices);
    tft.print(")");
    
    tft.drawFastHLine(0, 35, 240, isWifi ? C_GREEN : C_BLUE);
    
    tft.fillRect(0, 290, 240, 30, C_PANEL);
    uint16_t btnColor = isWifi ? C_GREEN : C_BLUE;
    tft.drawRoundRect(15, 293, 60, 22, 4, btnColor);
    tft.drawRoundRect(90, 293, 60, 22, 4, btnColor);
    tft.drawRoundRect(165, 293, 60, 22, 4, btnColor);
    
    tft.setTextColor(C_WHITE, C_PANEL);
    tft.setCursor(35, 299); tft.print("UP");
    tft.setCursor(105, 299); tft.print("BACK");
    tft.setCursor(180, 299); tft.print("DOWN");
    
    tft.drawFastHLine(0, 290, 240, btnColor);
}

void drawWifiList() {
    int count = wifiSniffer.getNetworkCount();
    drawListHeader(true);
    
    tft.setTextSize(1);
    int maxVisible = 11;
    int startIdx = game.listScroll;
    
    for (int i = 0; i < maxVisible && (startIdx + i) < count; i++) {
        NetworkInfo* net = wifiSniffer.getNetwork(startIdx + i);
        if (!net) continue;
        
        int y = 40 + (i * 22);
        if (i % 2 == 0) tft.fillRect(0, y, 240, 21, C_DARK);
        
        if (net->hasHandshake) tft.fillCircle(8, y + 10, 4, C_ORANGE);
        
        tft.setTextColor(net->hasHandshake ? C_ORANGE : C_GREEN, (i % 2 == 0) ? C_DARK : C_BG);
        tft.setCursor(18, y + 6);
        String ssid = String(net->ssid);
        if (ssid.length() > 20) ssid = ssid.substring(0, 17) + "...";
        if (ssid.length() == 0) ssid = "[Hidden]";
        tft.print(ssid);
        
        tft.setTextColor(C_GREY, (i % 2 == 0) ? C_DARK : C_BG);
        tft.setCursor(200, y + 6);
        tft.print("CH");
        tft.print(net->channel);
    }
}

void drawBleList() {
    // BLE List removed for stateless scanning
    drawListHeader(false);
    tft.setTextColor(C_BLUE, C_BG);
    tft.setCursor(60, 150);
    tft.print("BLE TRACKING DISABLED");
    tft.setCursor(55, 170);
    tft.print("Attacks Only Mode");
}

// =============================================================================
// MENU VIEW
// =============================================================================
void drawMenu() {
    tft.fillScreen(C_BG);
    
    // Header with NETS button
    tft.fillRect(0, 0, 240, 35, C_PANEL);
    tft.setTextColor(petStats.getStageColor(), C_PANEL);
    tft.setTextSize(2);
    tft.setCursor(80, 10);
    tft.print("MENU");
    
    // NETS button (top right)
    tft.fillRoundRect(180, 5, 55, 25, 4, C_GREEN);
    tft.setTextColor(C_BG, C_GREEN);
    tft.setTextSize(1);
    tft.setCursor(190, 12);
    tft.print("NETS");
    
    int y = 42;
    
    // Stats Section
    tft.setTextColor(C_CYAN, C_BG);
    tft.setCursor(10, y);
    tft.print("== STATS ==");
    y += 14;
    
    tft.setTextColor(C_WHITE, C_BG);
    tft.setCursor(10, y);
    tft.print("Lv.");
    tft.print(petStats.stats.level);
    tft.print(" ");
    tft.setTextColor(petStats.getStageColor(), C_BG);
    tft.print(petStats.getStageName());
    y += 12;
    
    tft.setTextColor(C_GREEN, C_BG);
    tft.setCursor(10, y);
    tft.print("HP:");
    tft.print(petStats.stats.hp);
    tft.setTextColor(C_CYAN, C_BG);
    tft.setCursor(70, y);
    tft.print("MP:");
    tft.print(petStats.stats.mp);
    tft.setTextColor(C_ORANGE, C_BG);
    tft.setCursor(130, y);
    tft.print("XP:");
    tft.print(petStats.stats.xp);
    tft.print("/");
    tft.print(petStats.stats.xpMax);
    y += 14;
    
    // Network stats
    tft.setTextColor(C_GREEN, C_BG);
    tft.setCursor(10, y);
    tft.print("WiFi:");
    tft.print(wifiSniffer.getNetworkCount());
    tft.setTextColor(C_BLUE, C_BG);
    tft.setCursor(70, y);
    tft.print("BLE:");
    tft.print(bleScanner.totalDevices);
    tft.setTextColor(C_ORANGE, C_BG);
    tft.setCursor(130, y);
    tft.print("HS:");
    tft.print(wifiSniffer.handshakeCount);
    y += 18;
    
    // Attacks Section - show all with availability (increased to 11 items to fill screen)
    uint16_t themeColor = petStats.getStageColor();
    tft.setTextColor(themeColor, C_BG);
    tft.setCursor(10, y);
    tft.print("== ATTACKS ==");
    y += 14;
    
    int stage = petStats.stats.stage;
    int visibleCount = 0;
    int startIdx = game.menuScroll;
    
    // Show 11 attacks to fill space down to buttons
    for (int i = 0; i < 11; i++) {
        int idx = startIdx + i;
        
        // Alternate background (full width)
        if (i % 2 == 0) {
            tft.fillRect(0, y - 1, 240, 13, C_DARK);
        } else {
            // Ensure non-alternating rows are cleared or BG
            tft.fillRect(0, y - 1, 240, 13, C_BG); 
        }

        if (idx < ABILITY_COUNT) {
            bool isUnlocked = (idx <= stage);
            bool canAfford = isUnlocked && (petStats.stats.mp >= abilities[idx].mpCost);
            
            uint16_t bg = (i % 2 == 0) ? C_DARK : C_BG;
            
            // Status indicator
            if (!isUnlocked) {
                tft.setTextColor(C_GREY, bg);
                tft.setCursor(5, y);
                tft.print("X");
            } else if (canAfford) {
                tft.setTextColor(themeColor, bg);
                tft.setCursor(5, y);
                tft.print("*");
            } else {
                tft.setTextColor(C_RED, bg);
                tft.setCursor(5, y);
                tft.print("-");
            }
            
            // Attack name (Unified Color)
            tft.setTextColor(isUnlocked ? themeColor : C_GREY, bg);
            tft.setCursor(15, y);
            tft.print(abilities[idx].name);
            
            // MP cost
            tft.setTextColor(isUnlocked ? C_WHITE : C_GREY, bg);
            tft.setCursor(200, y);
            tft.print(abilities[idx].mpCost);
        }
        
        y += 13;
    }
    
    // Fill any remaining space before buttons with BG
    if (y < 270) {
        tft.fillRect(0, y, 240, 270 - y, C_BG);
    }
    
    // Bottom buttons - simplified
    tft.fillRect(0, 270, 240, 50, C_PANEL);
    tft.setTextSize(1);
    
    // Back button
    tft.fillRoundRect(10, 280, 70, 28, 4, C_WHITE);
    tft.setTextColor(C_BG, C_WHITE);
    tft.setCursor(30, 290);
    tft.print("BACK");
    
    // Scroll (only if needed)
    if (ABILITY_COUNT > 11) { // Updated threshold
        tft.fillRoundRect(90, 280, 30, 28, 4, C_DARK);
        tft.drawRoundRect(90, 280, 30, 28, 4, themeColor);
        tft.setTextColor(themeColor, C_DARK);
        tft.setCursor(100, 290);
        tft.print("^");
        
        tft.fillRoundRect(125, 280, 30, 28, 4, C_DARK);
        tft.drawRoundRect(125, 280, 30, 28, 4, themeColor);
        tft.setTextColor(themeColor, C_DARK);
        tft.setCursor(135, 290);
        tft.print("v");
    }
    
    // Reset button
    tft.fillRoundRect(165, 280, 65, 28, 4, C_DARK);
    tft.drawRoundRect(165, 280, 65, 28, 4, C_RED);
    tft.setTextColor(C_RED, C_DARK);
    tft.setCursor(178, 290);
    tft.print("RESET");
}

void handleMenuTouch(int tx, int ty) {
    // Header NETS button (y < 40, x > 175)
    if (ty < 40 && tx > 175) {
        game.inMenuView = false;
        game.inListView = true;
        game.showingWifi = true;
        game.listScroll = 0;
        drawWifiList();
        return;
    }
    
    // Bottom buttons (y > 270)
    if (ty > 270) {
        if (tx < 85) {
            // BACK button
            game.inMenuView = false;
            drawBackground();
        } else if (tx >= 90 && tx < 120) {
            // UP scroll
            if (game.menuScroll > 0) {
                game.menuScroll--;
                drawMenu();
            }
        } else if (tx >= 125 && tx < 160) {
            // DOWN scroll
            if (game.menuScroll < ABILITY_COUNT - 11) {
                game.menuScroll++;
                drawMenu();
            }
        } else if (tx >= 165) {
            // RESET button - show confirmation
            tft.fillRect(40, 100, 160, 80, C_DARK);
            tft.drawRect(40, 100, 160, 80, C_RED);
            tft.setTextColor(C_WHITE, C_DARK);
            tft.setTextSize(1);
            tft.setCursor(60, 115);
            tft.print("RESET ALL DATA?");
            tft.setCursor(55, 135);
            tft.print("Tap again to confirm");
            
            delay(500);
            unsigned long timeout = millis() + 3000;
            while (millis() < timeout) {
                if (ts.touched()) {
                    fullReset();
                    game.inMenuView = false;
                    return;
                }
                delay(50);
            }
            drawMenu();
        }
    }
}

// =============================================================================
// TOUCH
// =============================================================================
void handleTouch(int tx, int ty) {
    static unsigned long lastTouch = 0;
    if (millis() - lastTouch < 150) return;
    lastTouch = millis();
    
    petStats.stats.lastActivity = millis();
    
    if (petStats.stats.isSleeping) {
        petStats.wake();
        setStatus("WAKING UP!", "Online", C_CYAN);
        creature.triggerAnimation(ANIM_HAPPY, 500);
        return;
    }
    
    if (game.inListView) {
        if (ty < 40) {
            game.listScroll = 0;
            // game.showingWifi = (tx < 120); // Disable BLE toggle
            game.showingWifi = true; // Always WiFi
            drawWifiList();
        } else if (ty > 285) {
            if (tx < 80) {
                game.listScroll = max(0, game.listScroll - 6);
            } else if (tx > 160) {
                // Only WiFi items matter now
                int maxItems = wifiSniffer.getNetworkCount();
                game.listScroll = min(game.listScroll + 6, max(0, maxItems - 11));
            } else {
                game.inListView = false;
                drawBackground();
                return;
            }
            drawWifiList();
        }
        return;
    }
    
    // Menu view
    if (game.inMenuView) {
        handleMenuTouch(tx, ty);
        return;
    }
    
    // Buttons
    if (ty >= BUTTON_Y) {
        if (tx >= 15 && tx < 85) {
            // FEED
            petStats.feed(20);
            petStats.addXP(XP_FEED);
            setStatus("FEEDING!", "+MP +HP", C_GREEN);
            creature.triggerAnimation(ANIM_EATING, 800);
        } else if (tx >= 85 && tx < 155) {
            // ATTACK - Execute random available ability
            int maxAbilities = petStats.stats.stage + 1;
            if (maxAbilities > ABILITY_COUNT) maxAbilities = ABILITY_COUNT;
            
            // Find affordable abilities
            int affordable[ABILITY_COUNT];
            int count = 0;
            for (int i = 0; i < maxAbilities; i++) {
                if (petStats.stats.mp >= abilities[i].mpCost) {
                    affordable[count++] = i;
                }
            }
            
            if (count > 0) {
                // Pick random affordable ability
                int pick = affordable[random(0, count)];
                petStats.stats.mp -= abilities[pick].mpCost;
                petStats.addXP(XP_AUTO_ATTACK * 2); // Bonus XP for manual attack
                
                // Animation based on attack type
                if (abilities[pick].type == 0) {
                    creature.triggerAnimation(ANIM_ATTACK, 800);
                } else {
                    creature.triggerAnimation(ANIM_SCANNING, 1200);
                }
                
                setStatus("ATTACK!", abilities[pick].name, abilities[pick].type == 0 ? C_RED : C_CYAN);
                game.topBarDirty = true;
            } else {
                setStatus("LOW MP!", "Need more power...", C_RED);
                creature.triggerAnimation(ANIM_CRITICAL, 500);
            }
        } else if (tx >= 155 && tx < 225) {
            // MENU
            game.inMenuView = true;
            game.menuScroll = 0;
            drawMenu();
        }
    }
    // Top bar - info
    else if (ty < TOP_BAR_H) {
        // Show stage info
        String info = String(petStats.getStageName());
        setStatus("STAGE INFO", info.c_str(), petStats.getStageColor());
    }
    // Ghost area
    else if (ty > TOP_BAR_H && ty < STATUS_BAR_Y) {
        bleScanner.startScan();
        setStatus("SCANNING...", "BLE Active", C_BLUE);
        creature.triggerAnimation(ANIM_SCANNING, 2000);
    }
}

// =============================================================================
// SNIFFER TASK
// =============================================================================
void snifferTask(void *p) {
    wifiSniffer.start();
    while (true) {
        wifiSniffer.loop();
        sdManager.processBuffer();
        vTaskDelay(50); // Increased delay to prevent UI stutters (Core contention)
    }
}

// =============================================================================
// SPLASH SCREEN - by Alex R.
// =============================================================================
// Splash progress bar replaced animated circle

void drawGhostMascot(int cx, int cy) {
    // Simple ghost shape - filled ellipse body
    tft.fillEllipse(cx, cy, 20, 25, C_GREEN);
    
    // Wavy bottom (3 bumps)
    for (int i = 0; i < 3; i++) {
        int bx = cx - 15 + (i * 15);
        tft.fillCircle(bx, cy + 22, 8, C_GREEN);
    }
    
    // Eyes
    tft.fillCircle(cx - 7, cy - 5, 4, C_BG);
    tft.fillCircle(cx + 7, cy - 5, 4, C_BG);
    tft.fillCircle(cx - 6, cy - 4, 2, C_CYAN);
    tft.fillCircle(cx + 8, cy - 4, 2, C_CYAN);
}

void drawProgressBar(int percent) {
    // Simple progress bar - FAST
    int barWidth = 160;
    int barHeight = 8;
    int x = (240 - barWidth) / 2;
    int y = 230;
    int fillWidth = (percent * barWidth) / 100;
    
    // Background
    tft.drawRect(x - 1, y - 1, barWidth + 2, barHeight + 2, C_DARK);
    
    // Fill
    if (fillWidth > 0) {
        tft.fillRect(x, y, fillWidth, barHeight, C_GREEN);
    }
}

void drawSplash() {
    tft.fillScreen(C_BG);
    
    // Ghost mascot centered
    drawGhostMascot(120, 100);
    
    // Title centered below mascot (Size 2 = ~12px/char, 9 chars = 108px width)
    // 240 - 108 = 132 / 2 = 66
    tft.setTextSize(2);
    tft.setTextColor(C_DARK, C_BG);
    tft.setCursor(68, 152);
    tft.print("NEONGHOST");
    tft.setTextColor(C_GREEN, C_BG);
    tft.setCursor(66, 150);
    tft.print("NEONGHOST");
    
    // Version (Size 1 = ~6px/char, 4 chars = 24px width)
    // 240 - 24 = 216 / 2 = 108
    tft.setTextSize(1);
    tft.setTextColor(C_CYAN, C_BG);
    tft.setCursor(108, 175);
    tft.print("v7.0");
    
    // Progress bar background
    tft.drawRect(39, 229, 162, 10, C_DARK);
    
    // 240 - 60 = 180 / 2 = 90
    tft.setTextColor(C_GREY, C_BG);
    tft.setCursor(90, 295);
    tft.print("by Alex R.");
    
    // Ready for progress updates
}

void drawDeathScreen() {
    tft.fillScreen(C_BG);
    
    // Header
    tft.fillRect(0, 0, 240, 50, C_RED);
    tft.setTextColor(C_WHITE, C_RED);
    tft.setTextSize(2);
    tft.setCursor(40, 18);
    tft.print("GHOST FAINTED");
    
    // Info
    tft.setTextSize(1);
    tft.setTextColor(C_WHITE, C_BG);
    tft.setCursor(30, 80);
    tft.print("System Critical Failure");
    tft.setCursor(55, 100);
    tft.print("Core Data Corrupted");
    
    // Revive Button
    tft.fillRoundRect(40, 220, 160, 60, 8, C_GREEN);
    tft.drawRoundRect(40, 220, 160, 60, 8, C_WHITE);
    tft.setTextColor(C_BG, C_GREEN);
    tft.setTextSize(2);
    tft.setCursor(85, 240);
    tft.print("REVIVE");
    
    // Warn
    tft.setTextSize(1);
    tft.setTextColor(C_RED, C_BG);
    tft.setCursor(55, 300);
    tft.print("Costs XP to Restore");
}

void updateSplashProgress(int percent, const char* status) {
    // Direct draw - NO animation loop
    drawProgressBar(percent);
    
    // Update status text centered
    tft.fillRect(20, 270, 200, 15, C_BG);
    tft.setTextSize(1);
    tft.setTextColor(C_GREY, C_BG);
    int textX = 120 - (strlen(status) * 3);
    tft.setCursor(textX, 270);
    tft.print(status);
}

// =============================================================================
// LED CONTROL
// =============================================================================
void setupLEDs() {
    // Config channels
    ledcSetup(0, 5000, 8); // Red
    ledcSetup(1, 5000, 8); // Green
    ledcSetup(2, 5000, 8); // Blue
    
    // Attach pins (CYD RGB is on 4, 16, 17)
    ledcAttachPin(CYD_LED_RED, 0);
    ledcAttachPin(CYD_LED_GREEN, 1);
    ledcAttachPin(CYD_LED_BLUE, 2);
    
    // Off (High = OFF for common anode/inverted logic on CYD?)
    // Most CYD back LEDs are Active LOW.
    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 255);
}

void updatePhysicalLED(uint16_t color) {
    // Convert RGB565 to RGB888
    uint8_t r = (color >> 8) & 0xF8;
    uint8_t g = (color >> 3) & 0xFC;
    uint8_t b = (color << 3) & 0xF8;
    
    // Active LOW (255 - Val)
    ledcWrite(0, 255 - r);
    ledcWrite(1, 255 - g);
    ledcWrite(2, 255 - b);
}

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n╔═════════════════════════════════════════╗");
    Serial.println("║     NEONGHOST v7.0 - HACKER PET         ║");
    Serial.println("╚═════════════════════════════════════════╝");
    
    // 1. Init display FIRST for instant feedback
    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);
    tft.begin();
    tft.setRotation(0);
    tft.invertDisplay(0);
    
    // 2. Show splash screen IMMEDIATELY
    drawSplash();
    updateSplashProgress(10, "DISPLAY OK");
    
    setupLEDs(); // Init RGB LED
    
    // 3. Init touch
    touchSPI.begin(CYD_TOUCH_CLK, CYD_TOUCH_MISO, CYD_TOUCH_MOSI, CYD_TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);
    updateSplashProgress(20, "TOUCH OK");
    
    // 4. Load saved data
    petStats.load();
    updateSplashProgress(30, "STATS LOADED");
    
    // 5. Init SD card
    sdManager.begin();
    updateSplashProgress(40, "SD READY");
    
    // 6. Init WiFi sniffer (SLOW)
    updateSplashProgress(50, "INIT WIFI...");
    wifiSniffer.init(&sdManager);
    wifiSniffer.setTechLevel(petStats.stats.level);
    updateSplashProgress(70, "WIFI READY");
    
    // 7. Start sniffer task
    xTaskCreatePinnedToCore(snifferTask, "Sniff", 10240, NULL, 1, NULL, 0); // Increased stack
    updateSplashProgress(80, "SNIFFER ACTIVE");
    
    // 8. Init BLE
    bleScanner.init();
    updateSplashProgress(90, "BLE READY");
    
    // 9. Draw main UI
    updateSplashProgress(100, "LAUNCHING...");
    delay(100); // Brief pause to show 100%
    
    if (petStats.stats.isDead) {
        drawDeathScreen();
        creature.triggerAnimation(ANIM_DEATH, 999999); // Stuck in death anim
    } else {
        drawBackground();
        setStatus("ONLINE", petStats.getAttackName(), petStats.getStageColor());
        creature.triggerAnimation(ANIM_HAPPY, 1000);
    }
    
    // 10. Start BLE scan
    bleScanner.startScan();
}

// =============================================================================
// LOOP
// =============================================================================
void loop() {
    static unsigned long lastFrame = 0;
    static bool wasDead = false;
    unsigned long now = millis();
    
    // DEATH HANDLING
    if (petStats.stats.isDead) {
        if (!wasDead) {
            drawDeathScreen();
            creature.triggerAnimation(ANIM_DEATH, 999999);
            wasDead = true;
        }
        
        // Only check for Revive touch
        if (ts.touched()) {
            TS_Point p = ts.getPoint();
            int tx = p.x;
            int ty = p.y;
            // Map 0-3800 to 0-320/240
            ty = map(ty, 200, 3800, 0, 320);
            tx = map(tx, 200, 3600, 0, 240); 
            
            // Limit checks
            if (tx < 0) tx = 0; if (tx > 240) tx = 240;
            if (ty < 0) ty = 0; if (ty > 320) ty = 320;
            
            // Revive button area: 40,220 -> 200,280
            if (tx > 40 && tx < 200 && ty > 220 && ty < 280) {
                petStats.revive();
                wasDead = false;
                drawBackground();
                creature.triggerAnimation(ANIM_EVOLVING, 2000); // Revive effect
                setStatus("REVIVED!", "System Restore Complete", C_GREEN);
            }
        }
        
        // Keep rendering ghost (dead animation)
        if (now - lastFrame > 33) {
            drawGhost();
            lastFrame = now;
        }
        return; // Skip rest of loop
    }
    
    petStats.update();
    wifiSniffer.setTechLevel(petStats.stats.level); // Unlock features based on level
    
    // WiFi events
    CaptureEvent evt = wifiSniffer.getNextEvent();
    if (evt != EVT_NONE) {
        String details = wifiSniffer.getEventDetails();
        switch (evt) {
            case EVT_HANDSHAKE:
                petStats.addXP(XP_HANDSHAKE);
                petStats.addHP(HP_HANDSHAKE);
                petStats.addMP(MP_HANDSHAKE);
                setStatus("HANDSHAKE!", details, C_ORANGE);
                // Big success!
                creature.triggerAnimation(ANIM_HAPPY, 2000);
                break;
            case EVT_DEAUTH:
                petStats.addXP(XP_DEAUTH);
                petStats.addHP(HP_CAPTURE);
                petStats.addMP(MP_CAPTURE);
                setStatus("DEAUTH!", details, C_RED);
                creature.triggerAnimation(ANIM_ATTACK, 1000);
                break;
            case EVT_NEW_NETWORK:
                petStats.addXP(XP_NEW_NETWORK);
                petStats.addHP(HP_CAPTURE);
                petStats.addMP(MP_CAPTURE);
                setStatus("NEW NET", details, C_GREEN);
                creature.triggerAnimation(ANIM_EATING, 600);
                break;
            case EVT_PROBE:
                petStats.addXP(XP_PROBE);
                creature.triggerAnimation(ANIM_SCANNING, 500);
                break;
            default: break;
        }
        game.topBarDirty = true;
    }
    
    // BLE
    bleScanner.loop();
    if (bleScanner.hasNewDevice()) {
        bleScanner.clearNewDevice();
        petStats.addXP(XP_BLE_DEVICE);
        petStats.addHP(HP_CAPTURE);
        petStats.addMP(MP_CAPTURE);
        setStatus("BLE FOUND", bleScanner.getLastDeviceName(), C_BLUE);
        creature.triggerAnimation(ANIM_HACKING, 800);
        game.topBarDirty = true;
    }
    
    // Periodic BLE (Only if no WiFi attack active)
    if (now - game.lastBleScan > 30000) {
        if (!wifiSniffer.isAttacking() && !creature.isAnimating()) {
            bleScanner.startScan();
            game.lastBleScan = now;
        }
    }
    
    // Cumulative abilities
    processAbilities();
    
    // Sleep
    if (petStats.stats.isSleeping && creature.currentAnim != ANIM_SLEEPING) {
        creature.triggerAnimation(ANIM_SLEEPING, 999999);
        setStatus("SLEEPING...", "Tap to wake", C_CYAN);
    }
    
    // Reset status
    if (!petStats.stats.isSleeping && now - game.alertStart > 4000) {
        if (game.statusLine1 != "SCANNING...") {
            setStatus("SCANNING...", petStats.getAttackName(), C_GREEN);
        }
    }
    
    // Auto-save
    if (now - game.lastSave > 300000) {
        petStats.save();
        game.lastSave = now;
    }
    
    // Touch
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        int tx = map(p.y, 3800, 200, 0, 240);
        int ty = map(p.x, 200, 3800, 0, 320);
        tx = constrain(tx, 0, 240);
        ty = constrain(ty, 0, 320);
        handleTouch(tx, ty);
    }
    
    // Render at 30fps
    if (now - lastFrame > 33) {
        if (!game.inListView && !game.inMenuView) {
            updateGhostPosition();
            drawGhost();
            drawTopBar();
            drawStatusBar();
        }
        lastFrame = now;
    }
}
