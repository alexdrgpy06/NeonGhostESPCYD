#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdint>

// Mock dependencies
struct NetworkInfo {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    bool hasHandshake;
};

class MockPacketSniffer {
public:
    int handshakeCount = 0;
    bool handshakeDetected = false;
    int pendingEvent = 0;
    std::string eventDetails;
    NetworkInfo networks[1];

    MockPacketSniffer() {
        memset(networks, 0, sizeof(networks));
    }

    int findNetwork(uint8_t* bssid) {
        return 0; // Always find network 0 for testing
    }

    // Current Implementation (O(N))
    void processPacket_Current(uint8_t *packet, uint16_t len) {
        if (len < 10) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;
        // uint8_t subtype = (frameControl >> 4) & 0x0F;

        if (type == 2) {
            for (int i = 24; i < len - 6 && i < 60; i++) {
                if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                    handshakeCount++;
                    handshakeDetected = true;
                    pendingEvent = 1;
                    eventDetails = "WPA HANDSHAKE";

                    uint8_t* bssid = &packet[16];
                    int idx = findNetwork(bssid);
                    if (idx >= 0) {
                        networks[idx].hasHandshake = true;
                    }
                    break;
                }
            }
        }
    }

    // Proposed Implementation (O(1))
    void processPacket_Optimized(uint8_t *packet, uint16_t len) {
        if (len < 24) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;

        if (type == 2) {
            // Protected bit (encrypted payload) - fast skip
            if (packet[1] & 0x40) return;

            uint8_t subtype = (frameControl >> 4) & 0x0F;
            uint16_t offset = 24; // Base header length

            // Check for Address 4 (ToDS=1 and FromDS=1)
            if ((packet[1] & 0x03) == 0x03) {
                offset += 6;
            }

            // Check for QoS Control (Data subtype bit 3 set)
            if (subtype & 0x08) {
                offset += 2;

                // Check for HT Control (Order bit set + QoS)
                if (packet[1] & 0x80) {
                    offset += 4;
                }
            }

            // Bounds check for EtherType (88 8e = EAPOL)
            if (len >= offset + 8) { // Minimum 8 bytes LLC header
                // Check LLC SNAP header (AA AA 03 00 00 00)
                if (packet[offset] == 0xAA && packet[offset+1] == 0xAA && packet[offset+2] == 0x03) {
                    if (packet[offset+6] == 0x88 && packet[offset+7] == 0x8E) {
                        handshakeCount++;
                        handshakeDetected = true;
                        pendingEvent = 1;
                        eventDetails = "WPA HANDSHAKE";

                        uint8_t* bssid = &packet[16];
                        int idx = findNetwork(bssid);
                        if (idx >= 0) {
                            networks[idx].hasHandshake = true;
                        }
                    }
                }
            }
        }
    }
};

void runBenchmark() {
    MockPacketSniffer sniffer;
    const int ITERATIONS = 5000000;

    // Test Case 1: Encrypted QoS Data frame (most common, ~95% of packets)
    uint8_t encPacket[100] = {0};
    encPacket[0] = 0x88; // QoS Data
    encPacket[1] = 0x41; // ToDS, Protected bit set
    encPacket[16] = 0x11; // BSSID
    encPacket[17] = 0x22;

    // Test Case 2: Plaintext EAPOL handshake (QoS Data)
    uint8_t eapolPacket[100] = {0};
    eapolPacket[0] = 0x88; // QoS Data
    eapolPacket[1] = 0x01; // ToDS, No Protected bit
    eapolPacket[16] = 0x11; // BSSID
    eapolPacket[17] = 0x22;
    // LLC + SNAP
    eapolPacket[26] = 0xAA;
    eapolPacket[27] = 0xAA;
    eapolPacket[28] = 0x03;
    eapolPacket[29] = 0x00;
    eapolPacket[30] = 0x00;
    eapolPacket[31] = 0x00;
    // EAPOL EtherType
    eapolPacket[32] = 0x88;
    eapolPacket[33] = 0x8E;

    // Benchmark Current - Encrypted
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; i++) {
        sniffer.processPacket_Current(encPacket, 100);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto current_enc_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Benchmark Optimized - Encrypted
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; i++) {
        sniffer.processPacket_Optimized(encPacket, 100);
    }
    end = std::chrono::high_resolution_clock::now();
    auto opt_enc_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Benchmark Current - EAPOL
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; i++) {
        sniffer.processPacket_Current(eapolPacket, 100);
    }
    end = std::chrono::high_resolution_clock::now();
    auto current_eapol_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Benchmark Optimized - EAPOL
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; i++) {
        sniffer.processPacket_Optimized(eapolPacket, 100);
    }
    end = std::chrono::high_resolution_clock::now();
    auto opt_eapol_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "=== PERFORMANCE BENCHMARK (" << ITERATIONS << " iterations) ===" << std::endl;
    std::cout << "Encrypted Frames (95% of traffic):" << std::endl;
    std::cout << "  Current (O(N)): " << current_enc_ms << " ms" << std::endl;
    std::cout << "  Optimized (O(1)): " << opt_enc_ms << " ms" << std::endl;
    if (opt_enc_ms > 0) std::cout << "  Speedup: " << (float)current_enc_ms / opt_enc_ms << "x" << std::endl;

    std::cout << "\nPlaintext EAPOL Frames:" << std::endl;
    std::cout << "  Current (O(N)): " << current_eapol_ms << " ms" << std::endl;
    std::cout << "  Optimized (O(1)): " << opt_eapol_ms << " ms" << std::endl;
    if (opt_eapol_ms > 0) std::cout << "  Speedup: " << (float)current_eapol_ms / opt_eapol_ms << "x" << std::endl;
}

int main() {
    runBenchmark();
    return 0;
}
