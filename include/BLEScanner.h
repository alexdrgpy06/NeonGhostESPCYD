#ifndef BLE_SCANNER_H
#define BLE_SCANNER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

#define MAX_BLE_DEVICES 64

struct BLEDeviceInfo {
    uint8_t mac[6];
    char name[20];
    int8_t rssi;
    unsigned long lastSeen;
};

class BLEScanner {
public:
    BLEScanner();
    void init();
    void startScan();  // Start a 3-second scan
    void loop();
    
    // Device tracking
    int getDeviceCount();
    BLEDeviceInfo* getDevice(int idx);
    
    // Events
    bool hasNewDevice();
    void clearNewDevice();
    String getLastDeviceName();
    
    // Stats
    volatile uint32_t totalDevices;
    volatile uint32_t scanCount;
    
private:
    NimBLEScan* pScan;
    BLEDeviceInfo devices[MAX_BLE_DEVICES];
    int deviceCount;
    
    bool scanning;
    bool newDeviceFlag;
    String lastDeviceName;
    unsigned long lastScanStart;
    
    int findDevice(uint8_t* mac);
    int addDevice(uint8_t* mac, const char* name, int8_t rssi);
    
    class ScanCallbacks : public NimBLEAdvertisedDeviceCallbacks {
    public:
        BLEScanner* parent;
        void onResult(NimBLEAdvertisedDevice* device) override;
    };
    ScanCallbacks callbacks;
};

#endif
