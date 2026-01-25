#include <Arduino.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#include "CreatureRenderer.h"
#include "PacketSniffer.h"
#include "SDManager.h"

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
#define C_GRID  0x0025 // Very dark blue
#define C_HUD   TFT_CYAN 
#define C_WARN  TFT_RED
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
void drawGrid() {
    tft.fillScreen(C_FONDO);
    for(int i=0; i<240; i+=20) tft.drawFastVLine(i, 0, 320, C_GRID);
    for(int i=0; i<320; i+=20) tft.drawFastHLine(0, i, 240, C_GRID);
}

void drawCyberButton(int x, int y, int w, int h, const char* label, uint16_t color) {
    tft.fillRoundRect(x, y, w, h, 6, 0x10A2);
    tft.drawRoundRect(x, y, w, h, 6, color);
    
    tft.setTextFont(4); // Larger Font (Font 4)
    tft.setTextSize(1);
    tft.setTextColor(color, 0x10A2);
    // Adjust Center String Y offset for Font 4
    tft.drawCentreString(label, x + w/2, y + (h/2) - 12, 4);
}

// --- UI LOGIC (Simplified - No Mutex) ---
void updateUI() {
      // 1. STATS BAR (Top)
      tft.fillRect(0, 0, 240, 40, C_FONDO); 
      tft.drawFastHLine(0, 40, 240, C_HUD);
      
      tft.setTextFont(4); // Switch to Font 4 for visibility
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
      
      // Line 2: Status (Centered) - Use Font 2 for status if long, or Font 4 if short
      tft.setTextFont(2);
      if(sniffer.hasHandshake()) {
          tft.setTextColor(TFT_YELLOW, C_FONDO);
          tft.drawCentreString("! EAPOL CAPTURED !", 120, 30, 2); // Keep Font 2 for long string or ensure it fits
      } else if (sniffer.hasDeauth()) {
          tft.setTextColor(TFT_RED, C_FONDO);
          tft.drawCentreString("JAMMING DETECTED", 120, 30, 2);
      } else {
          tft.setTextColor(0x5555, C_FONDO);
          tft.drawCentreString("SCANNING...", 120, 30, 2);
      }

      // 2. CREATURE RENDER
      renderer.draw(120, 160, miPet.dnaSeed, miPet.nivel);
      
      // 3. PACKET COUNTER
      tft.setTextFont(2);
      tft.setTextColor(0x2222, C_FONDO);
      tft.setCursor(80, 250);
      tft.print("PKTS: ");
      tft.print(sniffer.packetCount);
}

void evolver() {
  tft.fillScreen(TFT_WHITE); 
  delay(100);
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(C_NEON, C_FONDO);
  tft.setTextFont(4); // Large Font
  tft.drawCentreString("UPGRADE", 120, 140, 4);
  delay(2000);

  miPet.dnaSeed = esp_random();
  miPet.nivel++;
  miPet.xp = 0;
  miPet.xpNextLevel = miPet.nivel * 15;

  guardarProgreso();

  drawGrid();
  // Redraw buttons handled in main setup/loop part? No, redraw here:
  int btnY = 270; // Moved up to fit larger buttons
  drawCyberButton(5, btnY, 70, 40, "SCAN", C_HUD);
  drawCyberButton(85, btnY, 70, 40, "DATA", C_HUD);
  drawCyberButton(165, btnY, 70, 40, "SAVE", C_HUD);
}

// --- SNIFFER TASK (Task C: Multitasking) ---
// Runs on Core 0 to prevent UI blocking
void snifferTask(void *parameter) {
  sniffer.start();
  while (true) {
    sniffer.loop(); // Handles channel hopping
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
  drawGrid();
  int btnY = 270; // Moved up
  drawCyberButton(5, btnY, 70, 40, "SCAN", C_HUD);
  drawCyberButton(85, btnY, 70, 40, "DATA", C_HUD);
  drawCyberButton(165, btnY, 70, 40, "SAVE", C_HUD);
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
    
    if(y > 270) { // Button Area (Updated Y)
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
