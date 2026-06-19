/**
 * @file BTClassicManager.h
 * @brief Best-effort Bluetooth disruption module (speaker-focused).
 *
 * REALITY CHECK: A stock ESP32 cannot truly jam/deauth Bluetooth Classic
 * (BR/EDR) audio links, and NimBLE (used here) is BLE-only — real A2DP/AVRCP
 * interference would require a Bluedroid build variant and is not legal against
 * equipment you do not own. What this module actually does is *best-effort*
 * saturation of the 2.4GHz BLE advertising channels with rapidly rotating fake
 * device payloads, which can degrade nearby BLE pairing/scanning. It is framed
 * as "disruption", not a jammer. Authorized/educational use only.
 */
#ifndef BT_CLASSIC_MANAGER_H
#define BT_CLASSIC_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

class BTClassicManager {
public:
    void init();
    void startDisrupt(uint8_t intensity); // 1..5, scales rotation speed
    void stop();
    void loop();                          // rotate payloads while active
    bool active() const { return running; }

private:
    NimBLEAdvertising* pAdv = nullptr;
    bool running = false;
    uint8_t intensity = 1;
    unsigned long lastSwitch = 0;
    int idx = 0;

    void emitPayload(int i);
};

#endif // BT_CLASSIC_MANAGER_H
