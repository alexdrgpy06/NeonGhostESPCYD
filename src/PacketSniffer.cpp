#include "PacketSniffer.h"

// Global pointer for ISR callback access
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
}

void PacketSniffer::init(SDManager *sd) {
  sdManager = sd;
  globalSniffer = this;

  // Setup WiFi in Station Mode but disconnected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(false);
}

void PacketSniffer::start() {
  if (isRunning)
    return;

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
  if (!isRunning)
    return;

  // Task B: Channel Hopping
  // Switch WiFi channels (1-13) every 200ms
  unsigned long now = millis();
  if (now - lastChannelHop > 200) {
    lastChannelHop = now;
    currentChannel++;
    if (currentChannel > 13)
      currentChannel = 1;

    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  }
}

bool PacketSniffer::hasHandshake() { return handshakeDetected; }

void PacketSniffer::clearHandshake() { handshakeDetected = false; }

bool PacketSniffer::hasDeauth() { return deauthDetected; }

void PacketSniffer::clearDeauth() { deauthDetected = false; }

// Static Callback
void PacketSniffer::wifi_promiscuous_rx_cb(void *buf,
                                           wifi_promiscuous_pkt_type_t type) {
  if (globalSniffer) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    globalSniffer->processPacket(pkt->payload, pkt->rx_ctrl.sig_len);
  }
}

// Packet Processor (ISR Context - Must be fast!)
void PacketSniffer::processPacket(uint8_t *packet, uint16_t len) {
  packetCount++;
  if (len < 10)
    return; // Too short

  // Frame Control
  uint8_t frameControl = packet[0];
  uint8_t type = (frameControl >> 2) & 0x03;
  uint8_t subtype = (frameControl >> 4) & 0x0F;

  bool savePacket = false;

  // --- MANAGEMENT FRAMES (Type 0) ---
  if (type == 0) {
    if (subtype == 8) {
      // Beacon
      beaconCount++;
      // savePacket = true; // Save beacons? Optional, can fill buffer fast.
      // Let's only save EAPOL and Deauth to save space/bandwidth unless needed.
      // But prompt said "saving them to an SD Card... similar to ESP32 Marauder".
      // Marauder saves everything or filters.
      // Let's save beacons for now but beware of overflow.
      // Actually, beacons are huge volume. Let's SKIP beacons for PCAP to prioritize Handshakes.
      savePacket = false;
    } else if (subtype == 12) {
      // Deauthentication
      deauthCount++;
      deauthDetected = true;
      savePacket = true;
    }
  }

  // --- DATA FRAMES (Type 2) ---
  else if (type == 2) {
    // Task B: Packet Parsing (EAPOL Detection)
    // EtherType EAPOL = 0x888E
    // We scan the first 60 bytes to find the EtherType pattern
    for (int i = 24; i < len - 6 && i < 60; i++) {
      if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
        handshakeCount++;
        handshakeDetected = true; // Sets flag for Task B Game Logic
        savePacket = true;
        break;
      }
    }
  }

  // Pass captured packets to SDManager (Task A/B Integration)
  if (savePacket && sdManager) {
    sdManager->addPacket(packet, len);
  }
}
