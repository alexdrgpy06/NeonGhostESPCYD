/**
 * @file WiFiAttackManager.h
 * @brief On-demand WiFi recon + best-effort deauth (authorized use only).
 *
 * WiFi shares the single radio with BLE; the AttackManager switches RadioMode
 * before using this. Scanning is reliable; raw deauth TX via esp_wifi_80211_tx
 * is best-effort (newer ESP-IDF may drop deauth frames) and must only be used on
 * networks you are authorized to test.
 */
#ifndef WIFI_ATTACK_MANAGER_H
#define WIFI_ATTACK_MANAGER_H

#include <Arduino.h>

#define WIFI_MAX_APS 16

struct WiFiAP {
    char     ssid[33];
    uint8_t  bssid[6];
    int32_t  rssi;
    uint8_t  channel;
};

class WiFiAttackManager {
public:
    void init();
    void enableRadio();
    void disableRadio();
    bool radioOn() const { return on; }

    int  scan();                 // returns AP count found, fills list
    void deauth(uint8_t rounds); // best-effort broadcast deauth across found APs

    int  apCount() const { return count; }
    const WiFiAP& ap(int i) const { return aps[i]; }

private:
    bool on = false;
    WiFiAP aps[WIFI_MAX_APS];
    int count = 0;

    void sendDeauthFor(const WiFiAP& target);
};

#endif // WIFI_ATTACK_MANAGER_H
