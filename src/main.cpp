/**
 * Author: Alejandro Ramírez
 * Project: NeonGhostESPCYD
 * Logic: Core firmware for the NeonGhost cyber-pet on ESP32-CYD.
 *   Digimon-style branching evolution (5 archetypes x 10 stages + New Game Plus),
 *   automatic BT+BLE powers (manual selection steers the route), real-time
 *   creature rendering and a system state machine that drives the pet's reactions.
 *
 * AUTHORIZED / EDUCATIONAL USE ONLY. See README for legal + ESP32 radio limits.
 */
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>

#include "CreatureRenderer.h"
#include "SDManager.h"
#include "BLEScanner.h"
#include "GhostSprites.h"
#include "PetStats.h"
#include "EvolutionTree.h"
#include "AttackManager.h"

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
SDManager sdManager;
BLEScanner bleScanner;
PetStatsManager petStats;
AttackManager attackManager;

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

    bool inMenuView = false;
    int menuScroll = 0;

    // Ghost position (free movement)
    float ghostX = 120;
    float ghostY = 130;
    float ghostVelX = 0.5;

    // Dirty flags
    bool topBarDirty = true;
    bool statusDirty = true;

    // Previous values
    int lastLevel = -1;
    int lastXP = -1;
    int lastHP = -1;
    int lastMP = -1;
} game;

bool wantHatch = false; // set by a tap on the egg

void setStatus(String line1, String line2, uint16_t color);
void drawButtons();
void drawTopBar();
void drawStatusBar();
void drawBackground();
void drawMenu();
void fullReset();

// Powers list geometry inside the menu.
#define MENU_LIST_TOP   140
#define MENU_ROW_H      13
#define MENU_VISIBLE    9

// =============================================================================
// DRAWING (No Flicker)
// =============================================================================
void drawBackground() {
    tft.fillScreen(C_BG);
    uint16_t col = petStats.getArchetypeColor();

    tft.fillRect(0, 0, 240, TOP_BAR_H, C_PANEL);
    tft.drawFastHLine(0, TOP_BAR_H, 240, col);

    tft.fillRect(0, GHOST_AREA_Y, 240, GHOST_AREA_H, C_BG);

    tft.fillRect(0, STATUS_BAR_Y, 240, STATUS_BAR_H, C_PANEL);
    tft.drawFastHLine(0, STATUS_BAR_Y, 240, col);

    tft.fillRect(0, STATUS_BAR_Y + STATUS_BAR_H, 240, 320 - STATUS_BAR_Y - STATUS_BAR_H, C_PANEL);

    drawButtons();
    game.topBarDirty = true;
    game.statusDirty = true;
}

void drawButtons() {
    uint16_t btnColor = petStats.getArchetypeColor();
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

    tft.fillRect(0, 0, 240, TOP_BAR_H, C_PANEL);
    uint16_t col = petStats.getArchetypeColor();

    // Level box
    tft.fillRoundRect(4, 4, 42, 37, 4, C_DARK);
    tft.drawRoundRect(4, 4, 42, 37, 4, col);
    tft.setTextColor(col, C_DARK);
    tft.setTextSize(1);
    tft.setCursor(8, 8);
    tft.print("Lv.");
    tft.setTextSize(2);
    int lvX = petStats.stats.level < 10 ? 15 : 10;
    tft.setCursor(lvX, 22);
    tft.print(petStats.stats.level);

    // Archetype + stage
    tft.setTextSize(1);
    tft.setTextColor(col, C_PANEL);
    tft.setCursor(50, 32);
    if (petStats.isEgg()) {
        tft.print("EGG .ENC");
    } else {
        tft.print(petStats.getArchetypeName());
        tft.print(" ");
        tft.print(petStats.getPhaseName());
    }

    // XP bar
    int xpX = 50, xpY = 5, xpW = 105, xpH = 12;
    tft.fillRect(xpX, xpY, xpW, xpH, C_DARK);
    tft.drawRect(xpX, xpY, xpW, xpH, col);
    int xpFill = map(petStats.stats.xp, 0, petStats.stats.xpMax, 0, xpW - 4);
    if (xpFill > 0) tft.fillRect(xpX + 2, xpY + 2, xpFill, xpH - 4, col);

    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(xpX, xpY + 14);
    tft.print("XP ");
    tft.print(petStats.stats.xp);
    tft.print("/");
    tft.print(petStats.stats.xpMax);

    // HP bar
    int hpX = 160, hpY = 4;
    tft.setTextColor(C_RED, C_PANEL);
    tft.setCursor(hpX, hpY);
    tft.print("HP");
    tft.fillRect(hpX + 15, hpY, 60, 10, C_DARK);
    tft.drawRect(hpX + 15, hpY, 60, 10, C_RED);
    int hpFill = map(petStats.stats.hp, 0, 100, 0, 56);
    if (hpFill > 0) tft.fillRect(hpX + 17, hpY + 2, hpFill, 6, C_RED);

    // MP bar
    int mpX = 160, mpY = 18;
    tft.setTextColor(C_CYAN, C_PANEL);
    tft.setCursor(mpX, mpY);
    tft.print("MP");
    tft.fillRect(mpX + 15, mpY, 60, 10, C_DARK);
    tft.drawRect(mpX + 15, mpY, 60, 10, C_CYAN);
    int mpFill = map(petStats.stats.mp, 0, 100, 0, 56);
    if (mpFill > 0) tft.fillRect(mpX + 17, mpY + 2, mpFill, 6, C_CYAN);

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

    // Radio mode tag
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(200, STATUS_BAR_Y + 8);
    tft.print(attackManager.radio() == RADIO_WIFI ? "WIFI" : "BLE");

    int y2 = STATUS_BAR_Y + 32;
    tft.setTextColor(C_BLUE, C_PANEL);
    tft.setCursor(10, y2);
    tft.print("Dev:");
    tft.print(petStats.stats.devicesFound);

    tft.setTextColor(attackManager.busy() ? C_ORANGE : C_GREY, C_PANEL);
    tft.setCursor(90, y2);
    tft.print("ATK:");
    tft.print(attackManager.busy() ? "ON" : "OFF");

    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(150, y2);
    tft.print("Jmp:");
    tft.print(petStats.stats.lineJumps);

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
// GHOST MOVEMENT & RENDER
// =============================================================================
void updateGhostPosition() {
    game.ghostX += game.ghostVelX;
    if (game.ghostX < 64 || game.ghostX > 176) {
        game.ghostVelX = -game.ghostVelX;
    }
}

void drawGhost() {
    int centerX = (int)game.ghostX;
    int centerY = GHOST_AREA_Y + (GHOST_AREA_H / 2);

    static int prevSpriteX = -1;
    static int prevSpriteY = -1;

    int newSpriteX = centerX - 64;
    int newSpriteY = centerY - 64;
    if (newSpriteX < 0) newSpriteX = 0;
    if (newSpriteX > 112) newSpriteX = 112;

    if (prevSpriteX >= 0 && (prevSpriteX != newSpriteX || prevSpriteY != newSpriteY)) {
        if (prevSpriteX < newSpriteX) {
            tft.fillRect(prevSpriteX, prevSpriteY, newSpriteX - prevSpriteX, 128, C_BG);
        } else if (prevSpriteX > newSpriteX) {
            tft.fillRect(newSpriteX + 128, prevSpriteY, prevSpriteX - newSpriteX, 128, C_BG);
        }
    }

    if (petStats.isEgg()) {
        creature.drawEgg(centerX, centerY);
    } else {
        creature.draw(centerX, centerY, (uint8_t)petStats.getArchetype(),
                      petStats.getStage(), attackManager.busy());
    }

    prevSpriteX = newSpriteX;
    prevSpriteY = newSpriteY;

    updatePhysicalLED(creature.currentColor);
}

// =============================================================================
// RESET
// =============================================================================
void fullReset() {
    petStats.reset();
    bleScanner.clearNewDevice();
    petStats.save();
    setStatus("FULL RESET!", "All data cleared", C_RED);
    creature.triggerAnimation(ANIM_HAPPY, 1500);
    drawBackground();
}

// =============================================================================
// MENU VIEW (stats + powers list; tapping a power runs it manually = preference)
// =============================================================================
void drawMenu() {
    tft.fillScreen(C_BG);
    uint16_t themeColor = petStats.getArchetypeColor();

    tft.fillRect(0, 0, 240, 35, C_PANEL);
    tft.setTextColor(themeColor, C_PANEL);
    tft.setTextSize(2);
    tft.setCursor(80, 10);
    tft.print("MENU");

    int y = 42;
    // Identity
    tft.setTextSize(1);
    tft.setTextColor(C_WHITE, C_BG);
    tft.setCursor(10, y);
    tft.print("Lv.");
    tft.print(petStats.stats.level);
    tft.print(" ");
    tft.setTextColor(themeColor, C_BG);
    tft.print(petStats.getArchetypeName());
    tft.print(" Stage ");
    tft.print(petStats.getStage());
    y += 13;

    // Vitals
    tft.setTextColor(C_GREEN, C_BG);  tft.setCursor(10, y);  tft.print("HP:"); tft.print(petStats.stats.hp);
    tft.setTextColor(C_CYAN, C_BG);   tft.setCursor(70, y);  tft.print("MP:"); tft.print(petStats.stats.mp);
    tft.setTextColor(C_ORANGE, C_BG); tft.setCursor(130, y); tft.print("XP:"); tft.print(petStats.stats.xp);
    y += 13;

    // Affinities (what it does most)
    tft.setTextColor(C_GREY, C_BG); tft.setCursor(10, y); tft.print("Aff J/S/N/W: ");
    tft.print(petStats.stats.affinity[ROUTE_JAMMER]); tft.print("/");
    tft.print(petStats.stats.affinity[ROUTE_SPAMMER]); tft.print("/");
    tft.print(petStats.stats.affinity[ROUTE_SNIFFER]); tft.print("/");
    tft.print(petStats.stats.affinity[ROUTE_STRIKER]);
    y += 13;

    // Lifetime counters (NG+)
    tft.setTextColor(C_GREY, C_BG); tft.setCursor(10, y);
    tft.print("Atk:"); tft.print(petStats.stats.totalAttacks);
    tft.print(" Jmp:"); tft.print(petStats.stats.lineJumps);
    tft.print(" Up:"); tft.print(petStats.stats.powerupsUnlocked);
    tft.print(" Age:"); tft.print(petStats.stats.ageBoots);
    y += 16;

    tft.setTextColor(themeColor, C_BG);
    tft.setCursor(10, y);
    tft.print("== POWERS (tap to use) ==");

    // Powers list
    int ly = MENU_LIST_TOP;
    for (int i = 0; i < MENU_VISIBLE; i++) {
        int idx = game.menuScroll + i;
        uint16_t bg = (i % 2 == 0) ? C_DARK : C_BG;
        tft.fillRect(0, ly - 1, 240, MENU_ROW_H, bg);
        if (idx < (int)POWER_COUNT) {
            int cost = petStats.powerMpCost(idx);
            bool canAfford = petStats.stats.mp >= (uint8_t)cost;
            bool home = (POWERS[idx].home == petStats.getArchetype());

            tft.setTextColor(canAfford ? themeColor : C_RED, bg);
            tft.setCursor(5, ly);
            tft.print(canAfford ? "*" : "-");

            tft.setTextColor(home ? C_WHITE : themeColor, bg);
            tft.setCursor(15, ly);
            tft.print(POWERS[idx].name);

            // kind tag
            const char* tag = POWERS[idx].kind == PK_BTCLASSIC ? "BTc" :
                              POWERS[idx].kind == PK_WIFI ? "WiFi" :
                              POWERS[idx].kind == PK_BLE ? "BLE" : "Rec";
            tft.setTextColor(C_GREY, bg);
            tft.setCursor(150, ly);
            tft.print(tag);

            tft.setTextColor(C_WHITE, bg);
            tft.setCursor(190, ly);
            tft.print("M");
            tft.print(cost);
        }
        ly += MENU_ROW_H;
    }

    // Bottom buttons
    tft.fillRect(0, 270, 240, 50, C_PANEL);
    tft.setTextSize(1);

    tft.fillRoundRect(10, 280, 60, 28, 4, C_WHITE);
    tft.setTextColor(C_BG, C_WHITE);
    tft.setCursor(25, 290);
    tft.print("BACK");

    tft.fillRoundRect(80, 280, 30, 28, 4, C_DARK);
    tft.drawRoundRect(80, 280, 30, 28, 4, themeColor);
    tft.setTextColor(themeColor, C_DARK);
    tft.setCursor(90, 290);
    tft.print("^");

    tft.fillRoundRect(115, 280, 30, 28, 4, C_DARK);
    tft.drawRoundRect(115, 280, 30, 28, 4, themeColor);
    tft.setTextColor(themeColor, C_DARK);
    tft.setCursor(125, 290);
    tft.print("v");

    tft.fillRoundRect(165, 280, 65, 28, 4, C_DARK);
    tft.drawRoundRect(165, 280, 65, 28, 4, C_RED);
    tft.setTextColor(C_RED, C_DARK);
    tft.setCursor(178, 290);
    tft.print("RESET");
}

void handleMenuTouch(int tx, int ty) {
    // Power rows
    if (ty >= MENU_LIST_TOP && ty < MENU_LIST_TOP + MENU_VISIBLE * MENU_ROW_H) {
        int row = (ty - MENU_LIST_TOP) / MENU_ROW_H;
        int idx = game.menuScroll + row;
        if (idx >= 0 && idx < (int)POWER_COUNT) {
            if (attackManager.runPower((uint8_t)idx, true)) {
                setStatus("MANUAL", POWERS[idx].name, C_ORANGE);
            } else {
                setStatus("LOW MP!", POWERS[idx].name, C_RED);
            }
            drawMenu();
        }
        return;
    }

    if (ty > 270) {
        if (tx < 75) {
            game.inMenuView = false;
            drawBackground();
        } else if (tx >= 80 && tx < 110) {
            if (game.menuScroll > 0) { game.menuScroll--; drawMenu(); }
        } else if (tx >= 115 && tx < 145) {
            if (game.menuScroll < (int)POWER_COUNT - MENU_VISIBLE) { game.menuScroll++; drawMenu(); }
        } else if (tx >= 165) {
            // RESET confirmation
            tft.fillRect(40, 100, 160, 80, C_DARK);
            tft.drawRect(40, 100, 160, 80, C_RED);
            tft.setTextColor(C_WHITE, C_DARK);
            tft.setCursor(60, 115);
            tft.print("RESET ALL DATA?");
            tft.setCursor(55, 135);
            tft.print("Tap again to confirm");
            delay(500);
            unsigned long timeout = millis() + 3000;
            while (millis() < timeout) {
                if (ts.touched()) { fullReset(); game.inMenuView = false; return; }
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

    // Tapping the encrypted egg "brute-forces" it open.
    if (petStats.isEgg()) {
        wantHatch = true;
        setStatus("BRUTE FORCE", "Cracking core...", C_CYAN);
        return;
    }

    if (game.inMenuView) { handleMenuTouch(tx, ty); return; }

    if (ty >= BUTTON_Y) {
        if (tx >= 15 && tx < 85) {
            // FEED
            petStats.feed(20);
            petStats.addXP(XP_FEED);
            setStatus("FEEDING!", "+MP +HP", C_GREEN);
            creature.triggerAnimation(ANIM_EATING, 800);
            game.topBarDirty = true;
        } else if (tx >= 85 && tx < 155) {
            // ATTACK - run a random affordable power (auto picks a BT/BLE one)
            uint8_t pick = 2 + random(0, 9); // ids 2..10 = BT/BLE powers
            if (attackManager.runPower(pick, true)) {
                petStats.addXP(XP_AUTO_ATTACK * 2);
                setStatus("ATTACK!", attackManager.lastPowerName(), C_RED);
                game.topBarDirty = true;
            } else {
                setStatus("LOW MP!", "Need more power...", C_RED);
                creature.triggerAnimation(ANIM_CRITICAL, 500);
            }
        } else if (tx >= 155 && tx < 225) {
            game.inMenuView = true;
            game.menuScroll = 0;
            drawMenu();
        }
    } else if (ty < TOP_BAR_H) {
        setStatus("STAGE INFO", petStats.getArchetypeName(), petStats.getArchetypeColor());
    } else if (ty > TOP_BAR_H && ty < STATUS_BAR_Y) {
        bleScanner.startScan();
        setStatus("SCANNING...", "BLE Active", C_BLUE);
        creature.triggerAnimation(ANIM_SCANNING, 2000);
    }
}

// =============================================================================
// SPLASH / DEATH SCREENS
// =============================================================================
void drawGhostMascot(int cx, int cy) {
    tft.fillEllipse(cx, cy, 20, 25, C_GREEN);
    for (int i = 0; i < 3; i++) {
        int bx = cx - 15 + (i * 15);
        tft.fillCircle(bx, cy + 22, 8, C_GREEN);
    }
    tft.fillCircle(cx - 7, cy - 5, 4, C_BG);
    tft.fillCircle(cx + 7, cy - 5, 4, C_BG);
    tft.fillCircle(cx - 6, cy - 4, 2, C_CYAN);
    tft.fillCircle(cx + 8, cy - 4, 2, C_CYAN);
}

void drawProgressBar(int percent) {
    int barWidth = 160, barHeight = 8;
    int x = (240 - barWidth) / 2;
    int y = 230;
    int fillWidth = (percent * barWidth) / 100;
    tft.drawRect(x - 1, y - 1, barWidth + 2, barHeight + 2, C_DARK);
    if (fillWidth > 0) tft.fillRect(x, y, fillWidth, barHeight, C_GREEN);
}

void drawSplash() {
    tft.fillScreen(C_BG);
    drawGhostMascot(120, 100);
    tft.setTextSize(2);
    tft.setTextColor(C_DARK, C_BG);
    tft.setCursor(68, 152);
    tft.print("NEONGHOST");
    tft.setTextColor(C_GREEN, C_BG);
    tft.setCursor(66, 150);
    tft.print("NEONGHOST");
    tft.setTextSize(1);
    tft.setTextColor(C_CYAN, C_BG);
    tft.setCursor(102, 175);
    tft.print("DIGI v9.0");
    tft.drawRect(39, 229, 162, 10, C_DARK);
    tft.setTextColor(C_GREY, C_BG);
    tft.setCursor(90, 295);
    tft.print("by Alex R.");
}

void drawDeathScreen() {
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 240, 50, C_RED);
    tft.setTextColor(C_WHITE, C_RED);
    tft.setTextSize(2);
    tft.setCursor(40, 18);
    tft.print("GHOST FAINTED");
    tft.setTextSize(1);
    tft.setTextColor(C_WHITE, C_BG);
    tft.setCursor(30, 80);
    tft.print("System Critical Failure");
    tft.setCursor(55, 100);
    tft.print("Core Data Corrupted");
    tft.fillRoundRect(40, 220, 160, 60, 8, C_GREEN);
    tft.drawRoundRect(40, 220, 160, 60, 8, C_WHITE);
    tft.setTextColor(C_BG, C_GREEN);
    tft.setTextSize(2);
    tft.setCursor(85, 240);
    tft.print("REVIVE");
    tft.setTextSize(1);
    tft.setTextColor(C_RED, C_BG);
    tft.setCursor(55, 300);
    tft.print("Costs XP to Restore");
}

void updateSplashProgress(int percent, const char* status) {
    drawProgressBar(percent);
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
    ledcSetup(0, 5000, 8);
    ledcSetup(1, 5000, 8);
    ledcSetup(2, 5000, 8);
    ledcAttachPin(CYD_LED_RED, 0);
    ledcAttachPin(CYD_LED_GREEN, 1);
    ledcAttachPin(CYD_LED_BLUE, 2);
    ledcWrite(0, 255);
    ledcWrite(1, 255);
    ledcWrite(2, 255);
}

void updatePhysicalLED(uint16_t color) {
    uint8_t r = (color >> 8) & 0xF8;
    uint8_t g = (color >> 3) & 0xFC;
    uint8_t b = (color << 3) & 0xF8;
    ledcWrite(0, 255 - r);
    ledcWrite(1, 255 - g);
    ledcWrite(2, 255 - b);
}

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\nNEONGHOST DIGI v9.0 - branching cyber-pet (authorized use only)");

    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);
    tft.begin();
    tft.setRotation(0);
    tft.invertDisplay(0);

    drawSplash();
    updateSplashProgress(10, "DISPLAY OK");

    setupLEDs();

    touchSPI.begin(CYD_TOUCH_CLK, CYD_TOUCH_MISO, CYD_TOUCH_MOSI, CYD_TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);
    updateSplashProgress(20, "TOUCH OK");

    petStats.load();
    updateSplashProgress(30, "STATS LOADED");

    sdManager.begin();
    creature.setSDReady(sdManager.isReady); // use SD sprites when available
    updateSplashProgress(40, "SD READY");

    updateSplashProgress(50, "INIT BLE...");
    bleScanner.init();
    attackManager.init(&petStats, &bleScanner);
    updateSplashProgress(70, "BLE READY");

    updateSplashProgress(90, "SYSTEMS OK");
    updateSplashProgress(100, "LAUNCHING...");
    delay(100);

    if (petStats.stats.isDead) {
        drawDeathScreen();
        creature.triggerAnimation(ANIM_DEATH, 999999);
    } else {
        drawBackground();
        if (petStats.isEgg()) {
            setStatus("ENCRYPTED EGG", "Tap to brute-force", C_CYAN);
        } else {
            setStatus("ONLINE", petStats.getArchetypeName(), petStats.getArchetypeColor());
            creature.triggerAnimation(ANIM_HAPPY, 1000);
        }
    }

    bleScanner.startScan();
}

// =============================================================================
// LOOP
// =============================================================================
void loop() {
    static unsigned long lastFrame = 0;
    static bool wasDead = false;
    static bool wasBusy = false;
    unsigned long now = millis();

    // DEATH HANDLING
    if (petStats.stats.isDead) {
        if (!wasDead) {
            drawDeathScreen();
            creature.triggerAnimation(ANIM_DEATH, 999999);
            wasDead = true;
        }
        if (ts.touched()) {
            TS_Point p = ts.getPoint();
            int tx = map(p.x, 200, 3600, 0, 240);
            int ty = map(p.y, 200, 3800, 0, 320);
            tx = constrain(tx, 0, 240);
            ty = constrain(ty, 0, 320);
            if (tx > 40 && tx < 200 && ty > 220 && ty < 280) {
                petStats.revive();
                wasDead = false;
                drawBackground();
                creature.triggerAnimation(ANIM_EVOLVING, 2000);
                setStatus("REVIVED!", "System Restore Complete", C_GREEN);
            }
        }
        if (now - lastFrame > 33) { drawGhost(); lastFrame = now; }
        return;
    }

    petStats.update();

    // Phase 0: the egg hatches on interaction or after ~20s, reading entropy to
    // pick the initial archetype.
    static unsigned long lifeStart = 0;
    if (lifeStart == 0) lifeStart = now;
    if (petStats.isEgg() && (wantHatch || now - lifeStart > 20000)) {
        wantHatch = false;
        petStats.hatchEgg();
        petStats.justJumped = false;
        petStats.justEvolved = false;
        creature.triggerAnimation(ANIM_EVOLVING, 2500);
        drawBackground();
        setStatus("HATCHED!", petStats.getArchetypeName(), petStats.getArchetypeColor());
    }

    // BLE discovery rewards
    bleScanner.loop();
    if (bleScanner.hasNewDevice()) {
        bleScanner.clearNewDevice();
        petStats.addXP(XP_BLE_DEVICE);
        petStats.addHP(HP_BLE_FOUND);
        petStats.addMP(MP_BLE_FOUND);
        petStats.stats.devicesFound++;
        petStats.addAffinity(ROUTE_SNIFFER, 1); // discovering feeds the sniffer route
        setStatus("BLE FOUND", bleScanner.getLastDeviceName(), C_BLUE);
        creature.triggerAnimation(ANIM_HACKING, 800);
        game.topBarDirty = true;
    }

    // Attack orchestration (default auto BT+BLE + radio switching). Powers stay
    // dormant until the egg hatches and an archetype is assigned.
    if (!petStats.isEgg()) attackManager.loop();

    // React to attack state (pet turns red / trembles / strobe LED).
    bool busy = attackManager.busy();
    if (busy && !wasBusy) {
        creature.triggerAnimation(ANIM_ATTACK, 15000);
        creature.setLedFx(CreatureRenderer::LED_STROBE, C_RED, 15000);
        setStatus("ATTACK", attackManager.lastPowerName(), C_RED);
        game.statusDirty = true;
    } else if (!busy && wasBusy) {
        creature.triggerAnimation(ANIM_IDLE, 0);
        creature.setLedFx(CreatureRenderer::LED_SOLID, petStats.getArchetypeColor(), 0);
        game.statusDirty = true;
    }
    wasBusy = busy;

    // Evolution reactions
    if (petStats.justJumped) {
        petStats.justJumped = false;
        petStats.justEvolved = false;
        creature.triggerAnimation(ANIM_EVOLVING, 2500);
        setStatus("DIGIVOLVE!", petStats.getArchetypeName(), petStats.getArchetypeColor());
        drawBackground();
    } else if (petStats.justEvolved) {
        petStats.justEvolved = false;
        creature.triggerAnimation(ANIM_EVOLVING, 1800);
        setStatus("EVOLVED!", String("Stage ") + (int)petStats.getStage(), petStats.getArchetypeColor());
        game.topBarDirty = true;
    }

    // Sleep
    if (petStats.stats.isSleeping && creature.currentAnim != ANIM_SLEEPING) {
        creature.triggerAnimation(ANIM_SLEEPING, 999999);
        setStatus("SLEEPING...", "Tap to wake", C_CYAN);
    }

    // Reset status banner
    if (!petStats.stats.isSleeping && !busy && !petStats.isEgg() && now - game.alertStart > 4000) {
        if (game.statusLine1 != "SCANNING...") {
            setStatus("SCANNING...", petStats.getArchetypeName(), petStats.getArchetypeColor());
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
        if (!game.inMenuView) {
            updateGhostPosition();
            drawGhost();
            drawTopBar();
            drawStatusBar();
        }
        lastFrame = now;
    }
}
