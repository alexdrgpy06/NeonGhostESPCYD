#include "BLEScanner.h"

BLEScanner::BLEScanner() {
    pScan = nullptr;
    deviceCount = 0;
    totalDevices = 0;
    scanCount = 0;
    scanning = false;
    newDeviceFlag = false;
    lastScanStart = 0;
    memset(devices, 0, sizeof(devices));
}

void BLEScanner::init() {
    NimBLEDevice::init("");
    pScan = NimBLEDevice::getScan();
    
    callbacks.parent = this;
    pScan->setAdvertisedDeviceCallbacks(&callbacks, true);
    pScan->setActiveScan(false);  // Passive = less power
    pScan->setInterval(100);
    pScan->setWindow(99);
    
    Serial.println("[BLE] Initialized");
}

void BLEScanner::startScan() {
    if (scanning || !pScan) return;
    
    scanning = true;
    lastScanStart = millis();
    pScan->start(3, false);  // 3 second scan, non-blocking
    scanCount++;
    Serial.println("[BLE] Scan started");
}

void BLEScanner::loop() {
    if (!pScan) return;
    
    // Check if scan completed
    if (scanning && !pScan->isScanning()) {
        scanning = false;
        Serial.printf("[BLE] Scan complete. Devices: %d\n", deviceCount);
    }
}

int BLEScanner::getDeviceCount() {
    return deviceCount;
}

BLEDeviceInfo* BLEScanner::getDevice(int idx) {
    if (idx < 0 || idx >= deviceCount) return nullptr;
    return &devices[idx];
}

bool BLEScanner::hasNewDevice() {
    return newDeviceFlag;
}

void BLEScanner::clearNewDevice() {
    newDeviceFlag = false;
}

String BLEScanner::getLastDeviceName() {
    return lastDeviceName;
}

int BLEScanner::findDevice(uint8_t* mac) {
    for (int i = 0; i < deviceCount; i++) {
        if (memcmp(devices[i].mac, mac, 6) == 0) {
            return i;
        }
    }
    return -1;
}

int BLEScanner::addDevice(uint8_t* mac, const char* name, int8_t rssi) {
    if (deviceCount >= MAX_BLE_DEVICES) {
        // Replace oldest
        int oldest = 0;
        for (int i = 1; i < deviceCount; i++) {
            if (devices[i].lastSeen < devices[oldest].lastSeen) {
                oldest = i;
            }
        }
        memcpy(devices[oldest].mac, mac, 6);
        strncpy(devices[oldest].name, name, 19);
        devices[oldest].rssi = rssi;
        devices[oldest].lastSeen = millis();
        return oldest;
    }
    
    int idx = deviceCount++;
    memcpy(devices[idx].mac, mac, 6);
    strncpy(devices[idx].name, name, 19);
    devices[idx].name[19] = '\0';
    devices[idx].rssi = rssi;
    devices[idx].lastSeen = millis();
    totalDevices++;
    return idx;
}

void BLEScanner::ScanCallbacks::onResult(NimBLEAdvertisedDevice* device) {
    if (!parent) return;
    
    // Get MAC
    NimBLEAddress addr = device->getAddress();
    uint8_t mac[6];
    memcpy(mac, addr.getNative(), 6);
    
    // Get name
    std::string nameStr = device->getName();
    const char* name = nameStr.length() > 0 ? nameStr.c_str() : "Unknown";
    
    // Check if new
    int idx = parent->findDevice(mac);
    if (idx == -1) {
        parent->addDevice(mac, name, device->getRSSI());
        parent->newDeviceFlag = true;
        parent->lastDeviceName = String(name);
        Serial.printf("[BLE] New: %s\n", name);
    } else {
        // Update existing
        parent->devices[idx].rssi = device->getRSSI();
        parent->devices[idx].lastSeen = millis();
    }
}
