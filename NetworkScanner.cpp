#include "NetworkScanner.h"

NetworkScanner::NetworkScanner() {
  state = SCAN_IDLE;
  networksFound = 0;
  lastScanTime = 0;
}

void NetworkScanner::setup() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
}

void NetworkScanner::startScan() {
  if (state == SCANNING)
    return;

  // true = async scan
  WiFi.scanNetworks(true);
  state = SCANNING;
}

bool NetworkScanner::isScanning() { return state == SCANNING; }

int NetworkScanner::getResultCount() { return networksFound; }

void NetworkScanner::update() {
  if (state == SCANNING) {
    int status = WiFi.scanComplete();

    if (status >= 0) {
      networksFound = status;
      state = SCAN_COMPLETED;
    } else if (status == WIFI_SCAN_FAILED) {
      state = SCAN_FAILED;
    }
    // status -1 means still scanning
  }
}
