#ifndef PACKET_SNIFFER_H
#define PACKET_SNIFFER_H

#include <Arduino.h>
#include <esp_wifi.h>
#include "SDManager.h"
#include <queue>

// Structure to hold captured packet info for the queue
struct SniffedPacket {
    uint8_t* data;
    uint16_t len;
};

class PacketSniffer {
public:
    PacketSniffer();
    void init(SDManager* sdManager);

    // Check if a handshake was detected since last clear
    bool hasHandshake();
    void clearHandshake();

    // The task loop to be run on Core 0
    void snifferLoop();

    // Static callback for promiscuous mode
    static void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);

private:
    static SDManager* _sdManager;
    static QueueHandle_t _packetQueue;
    static bool _handshakeDetected;
};

// Global wrapper for the task
void snifferTaskWrapper(void* parameter);

#endif
