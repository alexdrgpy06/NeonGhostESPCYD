#include "PacketSniffer.h"

PacketSniffer *globalSniffer = NULL;

PacketSniffer::PacketSniffer() {
    sdManager = NULL;
    currentChannel = 1;
    lastChannelHop = 0;
    isRunning = false;
    handshakeDetected = false;
    deauthDetected = false;
    packetCount = 0;
    deauthCount = 0;
    beaconCount = 0;
    handshakeCount = 0;
    probeCount = 0;
    networkCount = 0;
    probeCount_arr = 0;
    pendingEvent = EVT_NONE;
}

void PacketSniffer::init(SDManager *sd) {
    sdManager = sd;
    globalSniffer = this;
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_promiscuous(false);
    
    // Clear network list
    memset(networks, 0, sizeof(networks));
    memset(probes, 0, sizeof(probes));
}

void PacketSniffer::start() {
    if (isRunning) return;
    
    esp_wifi_set_promiscuous_rx_cb(&PacketSniffer::wifi_promiscuous_rx_cb);
    esp_wifi_set_promiscuous(true);
    isRunning = true;
    Serial.println("[Sniffer] Started");
}

void PacketSniffer::stop() {
    esp_wifi_set_promiscuous(false);
    isRunning = false;
    Serial.println("[Sniffer] Stopped");
}

void PacketSniffer::loop() {
    if (!isRunning) return;
    
    // Channel hopping every 200ms
    unsigned long now = millis();
    if (now - lastChannelHop > 200) {
        lastChannelHop = now;
        currentChannel++;
        if (currentChannel > 13) currentChannel = 1;
        esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
}

// Event system
CaptureEvent PacketSniffer::getNextEvent() {
    CaptureEvent evt = pendingEvent;
    pendingEvent = EVT_NONE;
    return evt;
}

String PacketSniffer::getEventDetails() {
    return eventDetails;
}

int PacketSniffer::getNetworkCount() {
    return networkCount;
}

NetworkInfo* PacketSniffer::getNetwork(int idx) {
    if (idx < 0 || idx >= networkCount) return NULL;
    return &networks[idx];
}

// Legacy compatibility
bool PacketSniffer::hasHandshake() { return handshakeDetected; }
void PacketSniffer::clearHandshake() { handshakeDetected = false; }
bool PacketSniffer::hasDeauth() { return deauthDetected; }
void PacketSniffer::clearDeauth() { deauthDetected = false; }

// Find network by BSSID
int PacketSniffer::findNetwork(uint8_t* bssid) {
    for (int i = 0; i < networkCount; i++) {
        if (memcmp(networks[i].bssid, bssid, 6) == 0) {
            return i;
        }
    }
    return -1;
}

// Add new network
int PacketSniffer::addNetwork(uint8_t* bssid, char* ssid, int8_t rssi, uint8_t ch) {
    if (networkCount >= MAX_NETWORKS) return -1;
    
    int idx = networkCount++;
    memcpy(networks[idx].bssid, bssid, 6);
    strncpy(networks[idx].ssid, ssid, 32);
    networks[idx].ssid[32] = '\0';
    networks[idx].rssi = rssi;
    networks[idx].channel = ch;
    networks[idx].hasHandshake = false;
    
    return idx;
}

// Parse beacon frame
void PacketSniffer::parseBeacon(uint8_t* packet, uint16_t len) {
    if (len < 38) return;
    
    uint8_t* bssid = &packet[16];  // BSSID at offset 16
    uint8_t* ssidTag = &packet[36]; // SSID tag starts at 36
    
    char ssid[33] = {0};
    if (ssidTag[0] == 0) { // Tag type 0 = SSID
        uint8_t ssidLen = ssidTag[1];
        if (ssidLen > 32) ssidLen = 32;
        memcpy(ssid, &ssidTag[2], ssidLen);
    }
    
    // Check if already known
    int idx = findNetwork(bssid);
    if (idx == -1) {
        // New network!
        addNetwork(bssid, ssid, -50, currentChannel);
        pendingEvent = EVT_NEW_NETWORK;
        eventDetails = String(ssid);
        Serial.print("[WiFi] New: ");
        Serial.println(ssid);
    }
}

// Parse probe request
void PacketSniffer::parseProbe(uint8_t* packet, uint16_t len) {
    if (len < 26) return;
    
    uint8_t* srcMac = &packet[10];
    uint8_t* ssidTag = &packet[24];
    
    char ssid[33] = {0};
    if (ssidTag[0] == 0 && ssidTag[1] > 0) {
        uint8_t ssidLen = ssidTag[1];
        if (ssidLen > 32) ssidLen = 32;
        memcpy(ssid, &ssidTag[2], ssidLen);
    }
    
    if (strlen(ssid) > 0) {
        probeCount++;
        pendingEvent = EVT_PROBE;
        eventDetails = String(ssid);
    }
}

// Static callback
void PacketSniffer::wifi_promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
    if (globalSniffer) {
        wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
        globalSniffer->processPacket(pkt->payload, pkt->rx_ctrl.sig_len);
    }
}

// Main packet processor
void PacketSniffer::processPacket(uint8_t *packet, uint16_t len) {
    packetCount++;
    if (len < 10) return;
    
    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;
    uint8_t subtype = (frameControl >> 4) & 0x0F;
    
    bool savePacket = false;
    
    // MANAGEMENT FRAMES (Type 0)
    if (type == 0) {
        switch (subtype) {
            case 4:  // Probe Request
                parseProbe(packet, len);
                break;
                
            case 8:  // Beacon
                beaconCount++;
                parseBeacon(packet, len);
                break;
                
            case 12: // Deauthentication
                deauthCount++;
                deauthDetected = true;
                pendingEvent = EVT_DEAUTH;
                eventDetails = "DEAUTH ATTACK";
                savePacket = true;
                break;
        }
    }
    
    // DATA FRAMES (Type 2) - EAPOL Detection
    else if (type == 2) {
        for (int i = 24; i < len - 6 && i < 60; i++) {
            if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                handshakeCount++;
                handshakeDetected = true;
                pendingEvent = EVT_HANDSHAKE;
                eventDetails = "WPA HANDSHAKE";
                savePacket = true;
                
                // Mark network as having handshake
                uint8_t* bssid = &packet[16];
                int idx = findNetwork(bssid);
                if (idx >= 0) {
                    networks[idx].hasHandshake = true;
                }
                break;
            }
        }
    }
    
    // Save important packets
    if (savePacket && sdManager) {
        sdManager->addPacket(packet, len);
    }
}
