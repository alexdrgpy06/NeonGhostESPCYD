#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Preferences.h>

#include "CreatureRenderer.h"
#include "PacketSniffer.h"
#include "SDManager.h"
#include "BLEScanner.h"

// =============================================================================
// HARDWARE
// =============================================================================
#define CYD_TFT_BL     21
#define CYD_TOUCH_CS   33
#define CYD_TOUCH_IRQ  36
#define CYD_TOUCH_MOSI 32
#define CYD_TOUCH_MISO 39
#define CYD_TOUCH_CLK  25

// =============================================================================
// COLORS - Cyberpunk Theme
// =============================================================================
#define C_BG       0x0000
#define C_GRID     0x0208
#define C_PANEL    0x18C3
#define C_GREEN    0x07E0
#define C_CYAN     0x07FF
#define C_ORANGE   0xFD20
#define C_MAGENTA  0xF81F
#define C_RED      0xF800
#define C_BLUE     0x001F
#define C_WHITE    0xFFFF
#define C_GREY     0x4208
#define C_DARK     0x2104

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================
void drawBackground();
void setStatus(String line1, String line2, uint16_t color);
void drawWifiList();
void drawBleList();

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

// =============================================================================
// SCREEN MODES
// =============================================================================
enum ScreenMode {
    SCREEN_MAIN,      // Normal ghost view
    SCREEN_WIFI_LIST, // WiFi network list
    SCREEN_BLE_LIST   // BLE device list
};

// =============================================================================
// STATE
// =============================================================================
struct GameState {
    int level = 1;
    int xp = 0;
    int xpMax = 100;
    
    String statusLine1 = "";
    String statusLine2 = "";
    uint16_t themeColor = C_GREEN;
    
    // Screen mode
    ScreenMode screen = SCREEN_MAIN;
    int listScroll = 0;
    
    // Cache for dirty detection
    int lastLevel = -1;
    int lastXP = -1;
    uint32_t lastWifiNets = 0;
    uint32_t lastBleDevs = 0;
    uint32_t lastHS = 0;
    String lastStatus = "";
    
    unsigned long alertStart = 0;
    unsigned long lastSave = 0;
    unsigned long lastBleScan = 0;
} game;

// Evolution
const char* STAGE_NAMES[] = {"SPARK", "BABY", "TEEN", "SPECTER", "DEMON"};

int getStage(int level) {
    if (level <= 3) return 0;
    if (level <= 6) return 1;
    if (level <= 9) return 2;
    if (level <= 14) return 3;
    return 4;
}

// XP rewards
const int XP_HANDSHAKE = 100;
const int XP_DEAUTH = 25;
const int XP_NEW_NETWORK = 10;
const int XP_BLE_DEVICE = 8;
const int XP_PROBE = 3;
const int XP_MANUAL = 20;

// =============================================================================
// PERSISTENCE
// =============================================================================
void saveGame() {
    prefs.begin("ghost", false);
    prefs.putInt("lvl", game.level);
    prefs.putInt("xp", game.xp);
    prefs.end();
    game.lastSave = millis();
}

void loadGame() {
    prefs.begin("ghost", true);
    game.level = prefs.getInt("lvl", 1);
    game.xp = prefs.getInt("xp", 0);
    game.xpMax = game.level * 50 + 50;
    prefs.end();
}

void resetGame() {
    game.level = 1;
    game.xp = 0;
    game.xpMax = 100;
    game.lastLevel = -1;  // Force UI refresh
    game.lastXP = -1;
    saveGame();
    drawBackground();
    setStatus("GAME RESET!", "LV1", C_MAGENTA);
    creature.triggerAnimation(ANIM_EVOLVING, 1500);
}

// =============================================================================
// GAME LOGIC
// =============================================================================
void setStatus(String line1, String line2, uint16_t color) {
    game.statusLine1 = line1;
    game.statusLine2 = line2;
    game.themeColor = color;
    game.alertStart = millis();
}

void addXP(int amount, String source) {
    game.xp += amount;
    
    if (game.xp >= game.xpMax) {
        int oldStage = getStage(game.level);
        game.level++;
        game.xp = game.xp - game.xpMax;
        game.xpMax = game.level * 50 + 50;
        int newStage = getStage(game.level);
        
        if (newStage != oldStage) {
            setStatus("EVOLUTION!", STAGE_NAMES[newStage], C_MAGENTA);
            creature.triggerAnimation(ANIM_EVOLVING, 2000);
        } else {
            setStatus("LEVEL UP!", "LV" + String(game.level), C_MAGENTA);
            creature.triggerAnimation(ANIM_HAPPY, 1000);
        }
        saveGame();
    }
}

// =============================================================================
// UI DRAWING
// =============================================================================
void drawBackground() {
    tft.fillScreen(C_BG);
    
    // Grid in center area
    for (int y = 55; y <= 205; y += 25) {
        tft.drawFastHLine(0, y, 240, C_GRID);
    }
    for (int x = 0; x <= 240; x += 40) {
        tft.drawFastVLine(x, 55, 150, C_GRID);
    }
    
    // Top panel (Level/XP)
    tft.fillRect(0, 0, 240, 50, C_PANEL);
    tft.drawFastHLine(0, 50, 240, C_CYAN);
    
    // Bottom panel
    tft.fillRect(0, 210, 240, 110, C_PANEL);
    tft.drawFastHLine(0, 210, 240, C_CYAN);
    
    // Buttons
    tft.drawRect(8, 275, 72, 38, C_CYAN);
    tft.drawRect(84, 275, 72, 38, C_CYAN);
    tft.drawRect(160, 275, 72, 38, C_CYAN);
    
    tft.setTextColor(C_WHITE, C_PANEL);
    tft.setTextSize(1);
    tft.setCursor(28, 289); tft.print("FEED");
    tft.setCursor(102, 289); tft.print("LIST");
    tft.setCursor(180, 289); tft.print("SAVE");
}

void updateTopBar() {
    bool changed = (game.level != game.lastLevel) || (game.xp != game.lastXP);
    if (!changed) return;
    
    // Clear top area
    tft.fillRect(5, 5, 230, 40, C_PANEL);
    
    // LEVEL BADGE
    tft.fillRoundRect(8, 8, 55, 35, 4, C_DARK);
    tft.drawRoundRect(8, 8, 55, 35, 4, game.themeColor);
    tft.setTextColor(game.themeColor, C_DARK);
    tft.setTextSize(2);
    tft.setCursor(15, 17);
    tft.print("L");
    if (game.level < 10) tft.print("0");
    tft.print(game.level);
    
    // XP BAR (with number inside)
    int barX = 70, barY = 10, barW = 160, barH = 22;
    
    // Outer frame
    tft.drawRect(barX, barY, barW, barH, game.themeColor);
    
    // Fill
    int fillW = map(game.xp, 0, game.xpMax, 0, barW - 4);
    if (fillW > 0) {
        tft.fillRect(barX + 2, barY + 2, fillW, barH - 4, game.themeColor);
    }
    if (fillW < barW - 4) {
        tft.fillRect(barX + 2 + fillW, barY + 2, barW - 4 - fillW, barH - 4, C_DARK);
    }
    
    // XP Text INSIDE bar
    String xpText = String(game.xp) + "/" + String(game.xpMax);
    int tw = xpText.length() * 6;
    int textX = barX + (barW - tw) / 2;
    tft.setTextSize(1);
    tft.setTextColor(C_WHITE, fillW > (barW/2) ? game.themeColor : C_DARK);
    tft.setCursor(textX, barY + 7);
    tft.print(xpText);
    
    // Stage name
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(barX, barY + 26);
    tft.print(STAGE_NAMES[getStage(game.level)]);
    
    game.lastLevel = game.level;
    game.lastXP = game.xp;
}

void updateStatusBar() {
    if (game.statusLine1 == game.lastStatus) return;
    
    tft.fillRect(0, 213, 240, 30, C_PANEL);
    tft.setTextSize(1);
    
    // Line 1 (main status)
    tft.setTextColor(game.themeColor, C_PANEL);
    int tw1 = game.statusLine1.length() * 6;
    tft.setCursor((240 - tw1) / 2, 216);
    tft.print(game.statusLine1);
    
    // Line 2 (details)
    tft.setTextColor(C_WHITE, C_PANEL);
    int tw2 = game.statusLine2.length() * 6;
    tft.setCursor((240 - tw2) / 2, 228);
    tft.print(game.statusLine2);
    
    game.lastStatus = game.statusLine1;
}

void updateStatsBar() {
    int wifiNets = wifiSniffer.getNetworkCount();
    int bleDevs = bleScanner.getDeviceCount();
    
    bool changed = (wifiNets != (int)game.lastWifiNets) || 
                   (bleDevs != (int)game.lastBleDevs) ||
                   (wifiSniffer.handshakeCount != game.lastHS);
    if (!changed) return;
    
    tft.fillRect(5, 247, 230, 22, C_PANEL);
    tft.setTextSize(1);
    
    // WiFi stats
    tft.setTextColor(C_GREEN, C_PANEL);
    tft.setCursor(10, 252);
    tft.print("W:");
    tft.print(wifiNets);
    
    // BLE stats
    tft.setTextColor(C_BLUE, C_PANEL);
    tft.setCursor(55, 252);
    tft.print("B:");
    tft.print(bleDevs);
    
    // Handshakes
    tft.setTextColor(wifiSniffer.handshakeCount > 0 ? C_ORANGE : C_GREY, C_PANEL);
    tft.setCursor(100, 252);
    tft.print("HS:");
    tft.print(wifiSniffer.handshakeCount);
    
    // Packets
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(150, 252);
    tft.print("PKT:");
    tft.print(wifiSniffer.packetCount / 1000);
    tft.print("K");
    
    // Channel
    tft.setCursor(205, 252);
    tft.print("CH");
    tft.print(wifiSniffer.currentChannel);
    
    game.lastWifiNets = wifiNets;
    game.lastBleDevs = bleDevs;
    game.lastHS = wifiSniffer.handshakeCount;
}

// =============================================================================
// LIST VIEWS
// =============================================================================
void drawListHeader(bool isWifi) {
    int wifiCount = wifiSniffer.getNetworkCount();
    int bleCount = bleScanner.getDeviceCount();
    
    tft.fillScreen(C_BG);
    tft.fillRect(0, 0, 240, 35, C_PANEL);
    
    // Tab buttons
    // WiFi Tab
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
    tft.print(wifiCount);
    tft.print(")");
    
    // BLE Tab
    if (!isWifi) {
        tft.fillRect(125, 5, 110, 25, C_BLUE);
        tft.setTextColor(C_WHITE, C_BLUE);
    } else {
        tft.drawRect(125, 5, 110, 25, C_BLUE);
        tft.setTextColor(C_BLUE, C_PANEL);
    }
    tft.setCursor(145, 12);
    tft.print("BLE (");
    tft.print(bleCount);
    tft.print(")");
    
    tft.drawFastHLine(0, 35, 240, isWifi ? C_GREEN : C_BLUE);
    
    // Bottom bar
    tft.fillRect(0, 290, 240, 30, C_PANEL);
    tft.setTextColor(C_WHITE, C_PANEL);
    tft.setTextSize(1);
    tft.setCursor(100, 300);
    tft.print("[ BACK ]");
    tft.drawFastHLine(0, 290, 240, isWifi ? C_GREEN : C_BLUE);
    
    // Scroll buttons
    uint16_t btnColor = isWifi ? C_GREEN : C_BLUE;
    tft.drawRect(5, 293, 45, 22, btnColor);
    tft.setCursor(18, 299); tft.print("UP");
    tft.drawRect(190, 293, 45, 22, btnColor);
    tft.setCursor(198, 299); tft.print("DOWN");
}

void drawWifiList() {
    int count = wifiSniffer.getNetworkCount();
    drawListHeader(true);  // true = WiFi
    
    // List items
    tft.setTextSize(1);
    int maxVisible = 11;  // Adjusted for taller header
    int startIdx = game.listScroll;
    
    for (int i = 0; i < maxVisible && (startIdx + i) < count; i++) {
        NetworkInfo* net = wifiSniffer.getNetwork(startIdx + i);
        if (!net) continue;
        
        int y = 40 + (i * 22);
        
        // Background stripe
        if (i % 2 == 0) {
            tft.fillRect(0, y, 240, 21, C_DARK);
        }
        
        // Handshake indicator
        if (net->hasHandshake) {
            tft.fillCircle(8, y + 10, 4, C_ORANGE);
        }
        
        // SSID
        tft.setTextColor(net->hasHandshake ? C_ORANGE : C_GREEN, (i % 2 == 0) ? C_DARK : C_BG);
        tft.setCursor(18, y + 6);
        String ssid = String(net->ssid);
        if (ssid.length() > 20) ssid = ssid.substring(0, 17) + "...";
        if (ssid.length() == 0) ssid = "[Hidden]";
        tft.print(ssid);
        
        // Channel
        tft.setTextColor(C_GREY, (i % 2 == 0) ? C_DARK : C_BG);
        tft.setCursor(200, y + 6);
        tft.print("CH");
        tft.print(net->channel);
    }
    
    // Scroll indicator
    if (count > maxVisible) {
        tft.setTextColor(C_GREY, C_BG);
        tft.setCursor(180, 275);
        tft.print(startIdx + 1);
        tft.print("-");
        tft.print(min(startIdx + maxVisible, count));
        tft.print("/");
        tft.print(count);
    }
}

void drawBleList() {
    int count = bleScanner.getDeviceCount();
    drawListHeader(false);  // false = BLE
    
    tft.setTextSize(1);
    int maxVisible = 11;
    int startIdx = game.listScroll;
    
    for (int i = 0; i < maxVisible && (startIdx + i) < count; i++) {
        BLEDeviceInfo* dev = bleScanner.getDevice(startIdx + i);
        if (!dev) continue;
        
        int y = 40 + (i * 22);
        
        if (i % 2 == 0) {
            tft.fillRect(0, y, 240, 21, C_DARK);
        }
        
        // Name
        tft.setTextColor(C_CYAN, (i % 2 == 0) ? C_DARK : C_BG);
        tft.setCursor(10, y + 6);
        String name = String(dev->name);
        if (name.length() > 22) name = name.substring(0, 19) + "...";
        if (name.length() == 0) name = "Unknown";
        tft.print(name);
        
        // RSSI
        tft.setTextColor(C_GREY, (i % 2 == 0) ? C_DARK : C_BG);
        tft.setCursor(195, y + 6);
        tft.print(dev->rssi);
        tft.print("dB");
    }
    
    // Scroll indicator
    if (count > maxVisible) {
        tft.setTextColor(C_GREY, C_BG);
        tft.setCursor(180, 275);
        tft.print(startIdx + 1);
        tft.print("-");
        tft.print(min(startIdx + maxVisible, count));
        tft.print("/");
        tft.print(count);
    }
}

// =============================================================================
// TOUCH
// =============================================================================
void handleTouch(int tx, int ty) {
    static unsigned long lastTouch = 0;
    static unsigned long touchStart = 0;
    
    if (millis() - lastTouch < 150) return;
    lastTouch = millis();
    
    // List view touch handling
    if (game.screen != SCREEN_MAIN) {
        // TAB TOUCHES (header area: y < 40)
        if (ty < 40) {
            game.listScroll = 0;
            if (tx < 120) {
                // WiFi tab
                game.screen = SCREEN_WIFI_LIST;
                drawWifiList();
            } else {
                // BLE tab
                game.screen = SCREEN_BLE_LIST;
                drawBleList();
            }
            return;
        }
        
        // BOTTOM BAR (y > 285)
        if (ty > 285) {
            if (tx < 55) {
                // UP button
                game.listScroll = max(0, game.listScroll - 6);
                if (game.screen == SCREEN_WIFI_LIST) drawWifiList();
                else drawBleList();
            } else if (tx > 180) {
                // DOWN button
                int maxItems = (game.screen == SCREEN_WIFI_LIST) ? 
                    wifiSniffer.getNetworkCount() : bleScanner.getDeviceCount();
                game.listScroll = min(game.listScroll + 6, max(0, maxItems - 11));
                if (game.screen == SCREEN_WIFI_LIST) drawWifiList();
                else drawBleList();
            } else {
                // BACK button
                game.screen = SCREEN_MAIN;
                game.listScroll = 0;
                drawBackground();
            }
        }
        return;
    }
    
    // Main screen touch
    if (ty > 270) {
        if (tx < 80) {
            // FEED
            addXP(XP_MANUAL, "MANUAL");
            setStatus("FEEDING!", "+" + String(XP_MANUAL) + " XP", C_GREEN);
            creature.triggerAnimation(ANIM_EATING, 800);
        } else if (tx < 156) {
            // LIST button - cycle: Main -> WiFi -> BLE -> Main
            game.listScroll = 0;
            game.screen = SCREEN_WIFI_LIST;
            drawWifiList();
        } else {
            // SAVE
            saveGame();
            setStatus("SAVED!", "LV" + String(game.level), C_GREEN);
            creature.triggerAnimation(ANIM_HAPPY, 600);
        }
    }
    
    // Touch in ghost area
    else if (ty > 50 && ty < 210) {
        static int resetTaps = 0;
        static unsigned long lastResetTap = 0;
        
        if (game.screen != SCREEN_MAIN) return;
        
        // Check for reset sequence (3 rapid taps)
        if (millis() - lastResetTap < 500) {
            resetTaps++;
            if (resetTaps >= 3) {
                resetGame();
                resetTaps = 0;
            } else {
                setStatus("TAP " + String(3 - resetTaps) + " MORE", "TO RESET", C_RED);
            }
        } else {
            resetTaps = 1;
            // Normal function: BLE scan
            bleScanner.startScan();
            setStatus("BLE SCAN", "STARTED", C_BLUE);
            creature.triggerAnimation(ANIM_ALERT, 500);
        }
        lastResetTap = millis();
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
        vTaskDelay(10);
    }
}

// =============================================================================
// SETUP
// =============================================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n=== NeonGhost v2.1 ===");
    
    // Display
    pinMode(CYD_TFT_BL, OUTPUT);
    digitalWrite(CYD_TFT_BL, HIGH);
    tft.begin();
    tft.setRotation(0);
    tft.invertDisplay(0);
    
    // Touch
    touchSPI.begin(CYD_TOUCH_CLK, CYD_TOUCH_MISO, CYD_TOUCH_MOSI, CYD_TOUCH_CS);
    ts.begin(touchSPI);
    ts.setRotation(1);
    
    // SD & WiFi Sniffer
    sdManager.begin();
    wifiSniffer.init(&sdManager);
    xTaskCreatePinnedToCore(snifferTask, "Sniff", 8192, NULL, 1, NULL, 0);
    
    // BLE Scanner
    bleScanner.init();
    
    // Load game
    loadGame();
    
    // Draw UI
    drawBackground();
    setStatus("SYSTEM ONLINE", "WiFi + BLE", C_GREEN);
    
    // Initial BLE scan
    bleScanner.startScan();
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
    static unsigned long lastFrame = 0;
    unsigned long now = millis();
    
    // WiFi Sniffer events
    CaptureEvent evt = wifiSniffer.getNextEvent();
    if (evt != EVT_NONE) {
        String details = wifiSniffer.getEventDetails();
        switch (evt) {
            case EVT_HANDSHAKE:
                addXP(XP_HANDSHAKE, "HS");
                setStatus("! HANDSHAKE !", details, C_ORANGE);
                creature.triggerAnimation(ANIM_EATING, 1500);
                break;
            case EVT_DEAUTH:
                addXP(XP_DEAUTH, "DEAUTH");
                setStatus("DEAUTH", details, C_RED);
                creature.triggerAnimation(ANIM_ALERT, 1000);
                break;
            case EVT_NEW_NETWORK:
                addXP(XP_NEW_NETWORK, "WIFI");
                setStatus("NEW WIFI", details, C_GREEN);
                creature.triggerAnimation(ANIM_EATING, 600);
                break;
            case EVT_PROBE:
                addXP(XP_PROBE, "PROBE");
                creature.triggerAnimation(ANIM_EATING, 400);
                break;
            default: break;
        }
    }
    
    // BLE Scanner events
    bleScanner.loop();
    if (bleScanner.hasNewDevice()) {
        bleScanner.clearNewDevice();
        addXP(XP_BLE_DEVICE, "BLE");
        setStatus("NEW BLE", bleScanner.getLastDeviceName(), C_BLUE);
        creature.triggerAnimation(ANIM_EATING, 500);
    }
    
    // Periodic BLE scan (every 30 seconds)
    if (now - game.lastBleScan > 30000) {
        bleScanner.startScan();
        game.lastBleScan = now;
    }
    
    // Reset status after 4 seconds
    if (game.themeColor != C_GREEN && (now - game.alertStart > 4000)) {
        setStatus("SCANNING...", "CH" + String(wifiSniffer.currentChannel), C_GREEN);
    }
    
    // Auto-save every 5 minutes
    if (now - game.lastSave > 300000) {
        saveGame();
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
        // Check screen mode
        if (game.screen == SCREEN_MAIN) {
            // Ghost
            creature.draw(120, 130, game.level, CreatureRenderer::BABY);
            
            // UI updates
            updateTopBar();
            updateStatusBar();
            updateStatsBar();
        }
        // Lists are drawn on demand via touch handler, not every frame
        
        lastFrame = now;
    }
}
