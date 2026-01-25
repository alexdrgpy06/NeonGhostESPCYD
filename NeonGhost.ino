#include "CreatureRenderer.h"
#include "NetworkScanner.h"
#include "PacketSniffer.h"
#include "SDManager.h"
#include <Preferences.h> // Para guardar el progreso
#include <TFT_eSPI.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();
Preferences preferences;
CreatureRenderer renderer(&tft);
NetworkScanner scanner;
PacketSniffer sniffer;
SDManager sdManager;

// --- CONFIGURACIÓN VISUAL ---
#define C_FONDO TFT_BLACK
#define C_HUD 0x03E0 // Verde oscuro matrix
#define C_GLITCH TFT_MAGENTA

// --- ESTRUCTURA DE LA ENTIDAD ---
struct Daemon {
  unsigned long dnaSeed; // La "Semilla" que define su forma
  int nivel = 1;
  int xp = 0; // Comida (redes)
  int xpNextLevel = 10;
  String nombre = "UNK";
};

Daemon miPet;

// --- LÓGICA DEL JUEGO ---

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

  // Si es la primera vez (seed 0), generar uno nuevo
  if (miPet.dnaSeed == 0) {
    miPet.dnaSeed = esp_random();
    guardarProgreso();
  }
  preferences.end();
}

void drawUI() { // Prototipo
  tft.fillScreen(C_FONDO);

  // Marco HUD
  tft.drawRect(5, 5, 315, 235, C_HUD);

  // Info Stats
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, C_FONDO);
  tft.setCursor(15, 15);
  tft.print("LVL: ");
  tft.print(miPet.nivel);

  tft.setCursor(200, 15);
  tft.print("XP: ");
  tft.print(miPet.xp);
  tft.print("/");
  tft.print(miPet.xpNextLevel);

  // Botón FEED
  tft.fillRect(80, 180, 160, 40, C_HUD);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.drawString("HARVEST RF", 100, 192);

  // Dibujar al Bicho
  renderer.draw(160, 100, miPet.dnaSeed, miPet.nivel);
}

void evolucionar() {
  tft.fillScreen(C_FONDO);
  tft.setTextColor(TFT_YELLOW, C_FONDO);
  tft.setTextSize(3);
  tft.drawString("SYSTEM", 100, 80);
  tft.drawString("UPGRADE", 90, 110);
  delay(1000);

  // Mutación: La semilla cambia, creando un nuevo monstruo
  miPet.dnaSeed = esp_random();
  miPet.nivel++;
  miPet.xp = 0;
  miPet.xpNextLevel = miPet.nivel * 15; // Cuesta más subir cada vez

  guardarProgreso();

  tft.fillScreen(C_FONDO);
  drawUI();
}

void onScanComplete(int n) {
  // Cálculo de XP
  int ganancia = n * 5; // 5 XP por red
  if (n == 0)
    ganancia = 1; // Premio de consolación por ruido de fondo

  miPet.xp += ganancia;

  tft.fillRect(20, 190, 280, 30, C_FONDO);
  tft.setTextColor(TFT_GREEN, C_FONDO);
  String msg = "> ABSORBED " + String(n) + " NETS";
  tft.drawString(msg, 40, 200);

  delay(1500);

  // Chequeo de Nivel
  if (miPet.xp >= miPet.xpNextLevel) {
    evolucionar();
  } else {
    guardarProgreso();
    drawUI(); // Redibujar normal
  }
}

void onHandshakeCaptured() {
  // Massive XP for "Gourmet" filete
  miPet.xp += 100;

  // HACKING MODE Visuals
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_BLACK, TFT_RED);
  tft.setTextSize(3);
  tft.drawString("EAPOL DETECTED", 20, 100);
  tft.drawString("SAVING PCAP...", 30, 140);

  delay(2000); // Wait for user to see

  if (miPet.xp >= miPet.xpNextLevel) {
    evolucionar();
  } else {
    guardarProgreso();
    drawUI();
  }
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);

  // Initialize Hardware
  sdManager.begin(); // Uses default HSPI pins (14, 12, 13, 15)
  scanner.setup();

  // Initialize Sniffer logic
  sniffer.init(&sdManager);

  // Start Sniffer Task on Core 0 (Render is on Core 1 by default loop)
  xTaskCreatePinnedToCore(snifferTaskWrapper, "Sniffer", 10000, &sniffer, 1, NULL, 0);

  cargarProgreso();
  drawUI();
}

void loop() {
  uint16_t x, y;

  scanner.update();

  if (scanner.state == SCAN_COMPLETED) {
    onScanComplete(scanner.getResultCount());
    // Importante: Limpiar el scanner para la proxima
    scanner.state = SCAN_IDLE;
    WiFi.scanDelete();
  } else if (scanner.state == SCAN_FAILED) {
    scanner.state = SCAN_IDLE; // Reset simple
  }

  // Check for Sniffer Events
  if (sniffer.hasHandshake()) {
      sniffer.clearHandshake();
      onHandshakeCaptured();
  }

  if (tft.getTouch(&x, &y)) {
    if (x > 80 && x < 240 && y > 180 && y < 220) {
      if (!scanner.isScanning()) {
        tft.fillRect(20, 190, 280, 30, C_FONDO);
        tft.setTextColor(C_GLITCH, C_FONDO);
        tft.drawString("> SCANNING ETHER...", 40, 200);
        scanner.startScan();
      }
    }
  }

  delay(50); // Menor delay para UI más fluida
}
