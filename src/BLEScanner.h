#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

#define MAX_BLE_DEVICES 64 // Kept for reference or future use if needed

class BLEScanner {
public:
    BLEScanner();
    void init();
    void startScan();  // Start a 3-second scan
    void loop();
    
    // Device tracking (Stateless)
    // int getDeviceCount(); // Removed
    // BLEDeviceInfo* getDevice(int idx); // Removed
    
    // Events
    bool hasNewDevice();
    void clearNewDevice();
    String getLastDeviceName();
    
    // === BT CLASSIC ATTACKS ===
        void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
        void btClassicFlood();  // Mass advertising with fake names

        // === BT CLASSIC ATTACKS ===
            void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
            void btClassicFlood();  // Mass advertising with fake names

            // === BT CLASSIC ATTACKS ===
                void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
                void btClassicFlood();  // Mass advertising with fake names

                // === BT CLASSIC ATTACKS ===
                    void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
                    void btClassicFlood();  // Mass advertising with fake names

                    // === BT CLASSIC ATTACKS ===
                        void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
                        void btClassicFlood();  // Mass advertising with fake names

                        // === BT CLASSIC ATTACKS ===
                            void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
                            void btClassicFlood();  // Mass advertising with fake names

                            // === BT CLASSIC ATTACKS ===
                                void btClassicDeauth(); // Disruption for classic bluetooth devices (speakers)
                                void btClassicFlood();  // Mass advertising with fake names

                                // === BLE ATTACKS ===
                                void sourApple();      // Swift Pair spam for iOS
                                void samsungSpint();   // Samsung/Google Fast Pair
                                void bleFlood();       // Mass fake device advertising
                                void airTagSpam();     // Fake AirTag/FindMy beacons
                                void btDeauth();       // Bluetooth disruption (speaker focus)
                                void stopAttack();     // Stop any running attack

                                bool isAttacking();

                                // Stats
                                volatile uint32_t totalDevices;
                                volatile uint32_t scanCount;

                            private:
                                NimBLEScan* pScan;
                                // BLEDeviceInfo devices[MAX_BLE_DEVICES]; // Removed
                                // int deviceCount; // Removed

                                bool scanning;
                                bool newDeviceFlag;
                                String lastDeviceName;
                                unsigned long lastScanStart

                                // Attack state
                                NimBLEAdvertising* pAdvertising;
                                bool attackMode;
                                uint8_t attackType; // 0=None, 1=SourApple, 2=Samsung, 3=SwiftPair, 4=AirTag, 5=BTDeauth, 6=BTClassicDeauth, 7=BTClassicFlood
                                unsigned long attackStart;
                                unsigned long lastPayloadSwitch;
                                int payloadIndex;

                                // int findDevice(uint8_t* mac); // Removed
                                // int addDevice(uint8_t* mac, const char* name, int8_t rssi); // Removed

                                class ScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
                                public:
                                    BLEScanner* parent;
                                    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;
                                };
                                ScanCallbacks callbacks;
                            };
            bool newDeviceFlag;
            String lastDeviceName;
            unsigned long lastScanStart;
    
            // Attack state
            NimBLEAdvertising* pAdvertising;
            bool attackMode;
            uint8_t attackType; // 0=None, 1=SourApple, 2=Samsung, 3=SwiftPair, 4=AirTag, 5=BTDeauth, 6=BTClassicDeauth, 7=BTClassicFlood
            unsigned long attackStart;
            unsigned long lastPayloadSwitch;
            int payloadIndex;
    
            // int findDevice(uint8_t* mac); // Removed
            // int addDevice(uint8_t* mac, const char* name, int8_t rssi); // Removed
    
            class ScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
            public:
                BLEScanner* parent;
                void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;
            };
            ScanCallbacks callbacks;
        };

#endif