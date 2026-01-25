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

// Global variable for Task B/C integration
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
#define C_ALERT 0xF800 // Red

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
void drawCyberpunkGrid() {
    tft.fillScreen(C_FONDO);

    // Draw Grid
    tft.setTextColor(0x0040, C_FONDO); // Dark Green
    for (int i = 0; i < 320; i += 40) {
        tft.drawFastHLine(0, i, 240, 0x0020);
    }
    for (int i = 0; i < 240; i += 40) {
        tft.drawFastVLine(i, 0, 320, 0x0020);
    }
}

void drawDashboardStatic() {
    drawCyberpunkGrid();

    // Top Bar
    tft.fillRect(0, 0, 240, 30, 0x0000);
    tft.drawFastHLine(0, 30, 240, C_HUD);

    // Bottom Bar
    tft.fillRect(0, 270, 240, 50, 0x0000);
    tft.drawFastHLine(0, 270, 240, C_HUD);
}

void drawButtons() {
    tft.setSwapBytes(true);
    // Button Y position
    int y = 280;
    // Spacing
    int x1 = 30;
    int x2 = 104;
    int x3 = 178;

    // Draw Icons (32x32)
    tft.pushImage(x1, y, 32, 32, icon_scan);
    tft.pushImage(x2, y, 32, 32, icon_data);
    tft.pushImage(x3, y, 32, 32, icon_save);

    // Labels
    tft.setTextFont(1);
    tft.setTextColor(C_HUD, C_FONDO);
    tft.drawCentreString("SCAN", x1+16, y-10, 1);
    tft.drawCentreString("DATA", x2+16, y-10, 1);
    tft.drawCentreString("SAVE", x3+16, y-10, 1);
    tft.setSwapBytes(false);
}

// --- UI LOGIC ---
void updateUI() {
      tft.setTextFont(2);
      
      // 1. STATS (Top Bar)
      tft.setTextColor(C_HUD, C_FONDO);
      tft.setCursor(5, 5);
      tft.print("DAEMON: v");
      tft.print(miPet.nivel);
      tft.print(".0");

      // XP Bar
      int barWidth = 100;
      int fill = map(miPet.xp, 0, miPet.xpNextLevel, 0, barWidth);
      tft.drawRect(130, 8, barWidth, 10, C_HUD);
      tft.fillRect(132, 10, fill - 4, 6, C_NEON);

      // 2. ALERTS
      if(sniffer.hasHandshake()) {
          tft.setTextColor(C_FONDO, C_ALERT);
          tft.setTextFont(4);
          tft.drawCentreString("! EAPOL !", 120, 60, 4);
      } else if (sniffer.hasDeauth()) {
          tft.setTextColor(C_ALERT, C_FONDO);
          tft.drawCentreString("JAMMING...", 120, 50, 2);
      } else {
          // Clear area
          tft.fillRect(60, 50, 120, 30, C_FONDO);
      }

      // 3. CREATURE
      renderer.draw(120, 160, miPet.dnaSeed, miPet.nivel);
      
      // 4. PACKET STREAM
      tft.setTextFont(1);
      tft.setTextColor(0x5555, C_FONDO);
      tft.setCursor(10, 250);
      tft.print("RF_IN: ");
      tft.print(sniffer.packetCount);
      tft.print(" PKTS | CH: ");
      tft.print(sniffer.currentChannel);
}

void evolver() {
  tft.fillScreen(C_FONDO);
  tft.setTextColor(C_NEON, C_FONDO);
  tft.drawCentreString("EVOLUTION", 120, 140, 4);
  delay(1000);

  tft.setTextColor(TFT_WHITE, C_FONDO);
  tft.drawCentreString("SYSTEM UPGRADE", 120, 180, 2);
  delay(2000);

  miPet.dnaSeed = esp_random();
  miPet.nivel++;
  miPet.xp = 0;
  miPet.xpNextLevel = miPet.nivel * 15;

  guardarProgreso();

  drawDashboardStatic();
  drawButtons();
}

// --- SNIFFER TASK (Core 0) ---
void snifferTask(void *parameter) {
  sniffer.start();
  while (true) {
    sniffer.loop();
    sdManager.processBuffer(); // Move captured packets to SD
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// --- MAIN SETUP ---
void setup() {
  Serial.begin(115200);

  // BACKLIGHT
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);

  // TOUCH
  SPI.begin(14, 12, 13, 33); 
  ts.begin(); 
  ts.setRotation(0); 

  // TFT
  tft.begin();
  tft.setRotation(0); 
  // REMOVED: tft.invertDisplay(1); -> Fixes white screen issue

  // SD Check
  drawDashboardStatic();
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, C_FONDO);
  tft.drawCentreString("INIT SD...", 120, 160, 2);

  if (!sdManager.begin()) {
    tft.setTextColor(C_ALERT, C_FONDO);
    tft.drawCentreString("SD ERROR", 120, 180, 2);
    delay(1000);
  } else {
    tft.setTextColor(C_NEON, C_FONDO);
    tft.drawCentreString("SD MOUNTED", 120, 180, 2);
    delay(500);
  }
  
  // Clear Center
  tft.fillRect(40, 140, 160, 80, C_FONDO);

  sniffer.init(&sdManager);

  // Start Sniffer Task on Core 0
  xTaskCreatePinnedToCore(snifferTask, "Sniffer", 10000, NULL, 1, NULL, 0);

  cargarProgreso();

  // Initial UI Draw
  drawDashboardStatic();
  drawButtons();
}

void loop() {
  // Touch Handling
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    // Calibration for 2.8" CYD
    int y = map(p.x, 3800, 200, 0, 320); 
    int x = map(p.y, 3800, 200, 0, 240);
    
    if(y > 270) {
        if(x < 80) {
           Serial.println("CMD: SCAN TOGGLE");
           // Implement toggle logic if needed
        }
        else if(x < 160) Serial.println("CMD: VIEW DATA");
        else {
           Serial.println("CMD: NEW CAPTURE");
           sdManager.openNewPCAP();
           tft.setTextColor(C_NEON, C_FONDO);
           tft.drawCentreString("FILE SAVED", 120, 120, 2);
           delay(500);
           tft.fillRect(0, 110, 240, 30, C_FONDO);
        }
    }
  }

  // Game Logic
  if (sniffer.hasHandshake()) {
    sniffer.clearHandshake();
    handshakes_captured++;
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
  delay(50); // 20 FPS UI Update
}
