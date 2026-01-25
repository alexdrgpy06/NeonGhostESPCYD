#ifndef NETWORK_SCANNER_H
#define NETWORK_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>

enum ScannerState { SCAN_IDLE, SCANNING, SCAN_COMPLETED, SCAN_FAILED };

class NetworkScanner {
public:
  NetworkScanner();
  void setup();
  void update();
  void startScan();
  bool isScanning();
  int getResultCount();

  ScannerState state;
  int networksFound;

private:
  unsigned long lastScanTime;
  const unsigned long SCAN_INTERVAL = 5000; // Auto-scan optional logic
};

#endif
