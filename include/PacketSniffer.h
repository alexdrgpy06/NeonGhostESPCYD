#ifndef PACKET_SNIFFER_H
#define PACKET_SNIFFER_H

#include "SDManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Events
typedef enum { SNIFFER_EVENT_NONE, SNIFFER_EVENT_HANDSHAKE } SnifferEvent;

class PacketSniffer {
public:
  PacketSniffer();
  void init(SDManager *sd);
  void start();
  void stop();
  void loop(); // For channel hopping logic

  // Event Flags (Read & Clear)
  bool hasHandshake();
  void clearHandshake();

  bool hasDeauth();
  void clearDeauth();

  // Stats (Volatile for thread safety)
  volatile uint32_t packetCount;
  volatile uint32_t deauthCount;
  volatile uint32_t beaconCount;
  volatile uint32_t handshakeCount;

private:
  SDManager *sdManager;
  uint8_t currentChannel;
  unsigned long lastChannelHop;
  bool isRunning;

  // Handshake Flag (Volatile for ISR safety)
  volatile bool handshakeDetected;
  volatile bool deauthDetected;

  // Static Callback Wrapper
  static void wifi_promiscuous_rx_cb(void *buf,
                                     wifi_promiscuous_pkt_type_t type);

  // Instance Processor
  void processPacket(uint8_t *packet, uint16_t len);
};

#endif
