/**
 * @file BTClassicManager.cpp
 * @brief Best-effort 2.4GHz BLE-advertising saturation (see header for the
 *        honest limitations — this is NOT a real BR/EDR jammer).
 */
#include "BTClassicManager.h"

// A spread of fake manufacturer payloads (Apple / Microsoft / Samsung-ish) used
// to rapidly churn the advertising channels.
static const uint8_t MFG_IDS[][2] = {
    {0x4C, 0x00}, // Apple
    {0x06, 0x00}, // Microsoft
    {0x75, 0x00}, // Samsung
    {0xE0, 0x00}, // Google
    {0xFF, 0xFF}, // generic
};
#define MFG_COUNT (sizeof(MFG_IDS) / sizeof(MFG_IDS[0]))

void BTClassicManager::init() {
    pAdv = NimBLEDevice::getAdvertising();
}

void BTClassicManager::startDisrupt(uint8_t inten) {
    if (!pAdv) pAdv = NimBLEDevice::getAdvertising();
    intensity = constrain(inten, 1, 5);
    running = true;
    idx = 0;
    lastSwitch = 0;
    emitPayload(idx);
}

void BTClassicManager::stop() {
    running = false;
    if (pAdv) pAdv->stop();
}

void BTClassicManager::emitPayload(int i) {
    if (!pAdv) return;
    pAdv->stop();

    const uint8_t* mfg = MFG_IDS[i % MFG_COUNT];
    uint8_t payload[16];
    payload[0] = mfg[0];
    payload[1] = mfg[1];
    payload[2] = 0x02;
    payload[3] = 0x15;
    for (int b = 4; b < 16; b++) payload[b] = (uint8_t)random(0, 256);

    NimBLEAdvertisementData adv;
    adv.setManufacturerData(std::string((char*)payload, sizeof(payload)));
    pAdv->setAdvertisementData(adv);
    pAdv->start();
}

void BTClassicManager::loop() {
    if (!running) return;
    unsigned long now = millis();
    unsigned long interval = 220 - (intensity * 35); // faster at higher intensity
    if (now - lastSwitch >= interval) {
        idx++;
        emitPayload(idx);
        lastSwitch = now;
    }
}
