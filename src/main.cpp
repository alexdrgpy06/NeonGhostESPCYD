#include <Arduino.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#include "CreatureRenderer.h"
#include "PacketSniffer.h"
#include "SDManager.h"
#include "assets.h"

// --- OBJECTS ---
TFT_eSPI tft = TFT_eSPI();

// Global variable for Task B/C integration (Phase 2)
volatile uint32_t handshakes_captured = 0;

// TOUCH CONFIG
#define XPT_CS  33
#define XPT_IRQ 36 
XPT2046_Touchscreen ts(XPT_CS, XPT_IRQ);

Preferences preferences;
CreatureRenderer renderer(&tft);
PacketSniffer sniffer;
SDManager sdManager;

// --- CONFIGURACIÓN VISUAL ---
#define C_FONDO 0x0000 // Black
#define C_HUD   TFT_CYAN 
#define C_NEON  0x07E0 // Bright Green

// --- ESTRUCTURA DE LA ENTIDAD ---
struct Daemon {
  unsigned long dnaSeed;
  int nivel = 1;
  int xp = 0;
  int xpNextLevel = 10;
};
Daemon miPet;

// --- PERSISTENCE ---
void guardarProgreso() {
  preferences.begin("cyberpet", false);
  preferences.putULong("seed", miPet.dnaSeed);
  preferences.putInt("lvl", miPet.nivel);
  preferences.putInt("xp", miPet.xp);
  preferences.end();
}

void cargarProgreso() {
  preferences.begin("cyberpet", false);
  miPet.dnaSeed = preferences.getULong("seed", 0);
  miPet.nivel = preferences.getInt("lvl", 1);
  miPet.xp = preferences.getInt("xp", 0);

  if (miPet.dnaSeed == 0) {
    miPet.dnaSeed = esp_random();
    guardarProgreso();
  }
  preferences.end();
}

// --- UTILS ---
void drawBackground() {
    tft.setSwapBytes(true);
    tft.pushImage(0, 0, 240, 320, img_dashboard);
    tft.setSwapBytes(false);
}

void drawButtons() {
    tft.setSwapBytes(true);
    // Button Y position
    int y = 270;
    // Spacing
    int x1 = 30;
    int x2 = 104;
    int x3 = 178;

    // Draw Icons (32x32)
    tft.pushImage(x1, y, 32, 32, icon_scan);
    tft.pushImage(x2, y, 32, 32, icon_data);
    tft.pushImage(x3, y, 32, 32, icon_save);

    // Labels
    tft.setTextFont(2);
    tft.setTextColor(C_HUD, C_FONDO);
    tft.drawCentreString("SCAN", x1+16, y+35, 2);
    tft.drawCentreString("DATA", x2+16, y+35, 2);
    tft.drawCentreString("SAVE", x3+16, y+35, 2);
    tft.setSwapBytes(false);
}

// --- UI LOGIC ---
void updateUI() {
      // 1. STATS BAR (Top)
      tft.fillRect(0, 0, 240, 40, C_FONDO); 
      tft.drawFastHLine(0, 40, 240, C_HUD);
      
      tft.setTextFont(4);
      tft.setTextSize(1);
      tft.setTextColor(C_HUD, C_FONDO);
      
      // Line 1: LVL
      tft.setCursor(10, 8);
      tft.print("LVL ");
      tft.print(miPet.nivel);
      
      // Line 1: XP (Right)
      tft.setCursor(130, 8);
      tft.print("XP ");
      tft.print(miPet.xp);
      tft.print("/");
      tft.print(miPet.xpNextLevel);
      
      // Line 2: Status (Centered)
      if(sniffer.hasHandshake()) {
          tft.setSwapBytes(true);
          tft.pushImage(20, 50, 200, 50, img_alert_bg); // Overlay alert
          tft.setSwapBytes(false);
          tft.setTextColor(TFT_WHITE, TFT_RED);
          tft.drawCentreString("EAPOL CAPTURED", 120, 65, 4);
      } else if (sniffer.hasDeauth()) {
          tft.setTextColor(TFT_RED, C_FONDO);
          tft.drawCentreString("JAMMING DETECTED", 120, 50, 2);
      } else {
          tft.setTextColor(0x5555, C_FONDO);
          tft.drawCentreString("SCANNING...", 120, 50, 2);
      }

      // 2. CREATURE RENDER
      renderer.draw(120, 160, miPet.dnaSeed, miPet.nivel);
      
      // 3. PACKET COUNTER
      tft.setTextFont(2);
      tft.setTextColor(0x2222, C_FONDO);
      tft.setCursor(80, 240);
      tft.print("PKTS: ");
      tft.print(sniffer.packetCount);
}

void evolver() {
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 320, img_evolution);
  tft.setSwapBytes(false);
  
  tft.setTextColor(C_NEON, C_FONDO);
  tft.setTextFont(4);
  tft.drawCentreString("UPGRADE", 120, 140, 4);
  delay(2000);

  miPet.dnaSeed = esp_random();
  miPet.nivel++;
  miPet.xp = 0;
  miPet.xpNextLevel = miPet.nivel * 15;

  guardarProgreso();

  drawBackground();
  drawButtons();
}

// --- SNIFFER TASK (Task C: Multitasking) ---
// Runs on Core 0 to prevent UI blocking
void snifferTask(void *parameter) {
  sniffer.start();
  while (true) {
    sniffer.loop(); // Handles channel hopping

    // Process SD Buffer (Task Safe)
    // Moves data from RAM Ring Buffer to SD Card
    sdManager.processBuffer();

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// --- MAIN SETUP ---
void setup() {
  Serial.begin(115200);

  // BACKLIGHT
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);

  // TOUCH SPI (Manual)
  SPI.begin(14, 12, 13, 33); 
  ts.begin(); 
  ts.setRotation(0); 

  // TFT
  tft.begin();
  tft.setRotation(0); 
  tft.invertDisplay(1); // HARDWARE FIX: Invert colors

  // SD
  if (!sdManager.begin()) {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("NO SD", 120, 160, 4);
    delay(1000);
  }
  
  sniffer.init(&sdManager);
  xTaskCreatePinnedToCore(snifferTask, "Sniffer", 10000, NULL, 1, NULL, 0);

  cargarProgreso();

  // Initial Draw
  drawBackground();
  drawButtons();
}

void loop() {
  // Direct Touch Handling (No debouncing for test)
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    // Invert X/Y map for Portrait
    // Raw X: ~3800(left) to ~300(right)
    // Raw Y: ~3800(top) to ~300(bottom)
    int y = map(p.x, 3800, 200, 0, 320); 
    int x = map(p.y, 3800, 200, 0, 240);
    
    // VISUAL FEEDBACK (Hardware Test)
    tft.fillCircle(x, y, 3, TFT_WHITE);
    
    if(y > 270) {
        if(x < 80) Serial.println("BTN: SCAN");
        else if(x < 160) Serial.println("BTN: DATA");
        else Serial.println("BTN: SAVE");
    }
  }

  if (sniffer.hasHandshake()) {
    sniffer.clearHandshake();
    handshakes_captured++; // Feeds the global counter
    miPet.xp += 50; 
  }
  if (sniffer.hasDeauth()) {
    sniffer.clearDeauth();
    miPet.xp++;
  }

  if (miPet.xp >= miPet.xpNextLevel) {
    evolver();
  }

  updateUI();
  delay(33); 
}
