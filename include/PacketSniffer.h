#ifndef PACKET_SNIFFER_H
#define PACKET_SNIFFER_H

#include "SDManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Maximum tracked networks/devices
#define MAX_NETWORKS 64
#define MAX_PROBES 32

// Network info structure
struct NetworkInfo {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    bool hasHandshake;
};

// Probe request structure
struct ProbeInfo {
    uint8_t mac[6];
    char ssid[33];
    unsigned long timestamp;
};

// Capture event types
enum CaptureEvent {
    EVT_NONE = 0,
    EVT_HANDSHAKE,
    EVT_DEAUTH,
    EVT_PROBE,
    EVT_NEW_NETWORK,
    EVT_ATTACK
};

class PacketSniffer {
public:
    PacketSniffer();
    void init(SDManager *sd);
    void start();
    void stop();
    void loop();
    
    void setTechLevel(int level);
    void sendDeauth(uint8_t* bssid, uint8_t* client);

    // Event system
    CaptureEvent getNextEvent();
    String getEventDetails();
    
    // Network list
    int getNetworkCount();
    NetworkInfo* getNetwork(int idx);
    
    // Stats
    volatile uint32_t packetCount;
    volatile uint32_t deauthCount;
    
    // Attacks
    void startAttack(int type, int duration);
    void beaconSpam();     // Random SSIDs
    void rickRoll();       // Lyrics SSIDs
    void deauthAttack();   // Send deauth to random net
    void probeFlood();     // Probe request flood
    void authFlood();      // Authentication flood
    void stopAttack();     // Stop active attack
    bool isAttacking() { return attackMode; }
    
    volatile uint32_t beaconCount;
    volatile uint32_t handshakeCount;
    volatile uint32_t probeCount;
    volatile uint8_t currentChannel;
    
    // Legacy flags (for backwards compat)
    bool hasHandshake();
    void clearHandshake();
    bool hasDeauth();
    void clearDeauth();

private:
    SDManager *sdManager;
    unsigned long lastChannelHop;
    unsigned long lastAttack;
    bool isRunning;
    int techLevel;
    
    // Attack State
    bool attackMode;
    unsigned long attackStart;
    unsigned long attackDuration;
    int attackType; // 0=None, 1=Beacon, 2=RickRoll, 3=Deauth, 4=Probe
    
    void broadcastBeacon(const char* ssid);
    
    // Event queue
    volatile CaptureEvent pendingEvent;
    String eventDetails;
    
    // Tracking
    NetworkInfo networks[MAX_NETWORKS];
    int networkCount;
    ProbeInfo probes[MAX_PROBES];
    int probeCount_arr;
    
    // Flags
    volatile bool handshakeDetected;
    volatile bool deauthDetected;
    
    // Helpers
    int findNetwork(uint8_t* bssid);
    int addNetwork(uint8_t* bssid, char* ssid, int8_t rssi, uint8_t ch);
    void parseBeacon(uint8_t* packet, uint16_t len);
    void parseProbe(uint8_t* packet, uint16_t len);
    
    static void wifi_promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type);
    void processPacket(uint8_t *packet, uint16_t len);
};

#endif
