#include <iostream>
#include <cstdint>
#include <chrono>
#include <vector>

// Mock ESP32 definitions
#define MAX_NETWORKS 100
struct NetworkInfo {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    bool hasHandshake;
};

class PacketSnifferMock {
public:
    int handshakeCount = 0;
    NetworkInfo networks[MAX_NETWORKS];
    int networkCount = 0;

    int findNetwork(uint8_t* bssid) {
        for (int i = 0; i < networkCount; i++) {
            if (bssid[0] == networks[i].bssid[0] &&
                bssid[1] == networks[i].bssid[1] &&
                bssid[2] == networks[i].bssid[2] &&
                bssid[3] == networks[i].bssid[3] &&
                bssid[4] == networks[i].bssid[4] &&
                bssid[5] == networks[i].bssid[5]) {
                return i;
            }
        }
        return -1;
    }

    void processPacketLegacy(uint8_t *packet, uint16_t len) {
        if (len < 10) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;

        if (type == 2) {
            for (int i = 24; i < len - 6 && i < 60; i++) {
                if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                    handshakeCount++;
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

    void processPacketOptimized(uint8_t *packet, uint16_t len) {
        if (len < 10) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;

        if (type == 2) {
            // Memory check: The processPacket routine identifies EAPOL handshakes by checking the 'Protected' bit (`packet[1] & 0x40`) to skip encrypted frames, and uses direct O(1) offset calculations (accounting for Address 4, QoS, and HT Control) to locate the 0x888E ethertype, replacing linear scanning.
            if (packet[1] & 0x40) return; // Skip encrypted frames

            uint8_t subtype = (frameControl >> 4) & 0x0F;

            // Base offset is 24 bytes (Header + 3 MACs + Sequence Control)
            int offset = 24;

            // Check if ToDS and FromDS are both set (WDS), adding Address 4 (6 bytes)
            if ((packet[1] & 0x03) == 0x03) {
                offset += 6;
            }

            // QoS Control (2 bytes) is present if subtype bit 3 is set (e.g., 8)
            if (subtype & 0x08) {
                offset += 2;
            }

            // HT Control (4 bytes) is present if Order bit is set in Frame Control (packet[1] & 0x80)
            if (packet[1] & 0x80) {
                offset += 4;
            }

            // LLC/SNAP header (8 bytes) + Ethertype (2 bytes) = 10 bytes before Ethertype
            if (offset + 7 < len && packet[offset+6] == 0x88 && packet[offset+7] == 0x8E) {
                handshakeCount++;
                uint8_t* bssid = &packet[16];
                int idx = findNetwork(bssid);
                if (idx >= 0) {
                    networks[idx].hasHandshake = true;
                }
            }
        }
    }
};

int main() {
    PacketSnifferMock sniffer;
    sniffer.networkCount = 1; // Add one dummy network

    // Create an encrypted data frame
    uint8_t pkt_enc[100] = {0};
    pkt_enc[0] = 0x08; // Type 2 (Data), Subtype 0
    pkt_enc[1] = 0x40; // Protected bit set

    // Create a plain EAPOL frame
    uint8_t pkt_eapol[100] = {0};
    pkt_eapol[0] = 0x88; // Type 2 (Data), Subtype 8 (QoS Data)
    pkt_eapol[1] = 0x00; // Unprotected
    pkt_eapol[16] = 0; // dummy bssid
    // Header len = 24 (Base) + 2 (QoS) = 26
    // LLC starts at 26. Ethertype is at 26+6 = 32
    pkt_eapol[32] = 0x88;
    pkt_eapol[33] = 0x8E;

    // Create a plain data frame
    uint8_t pkt_plain[100] = {0};
    pkt_plain[0] = 0x08;
    pkt_plain[1] = 0x00;

    const int ITERATIONS = 10000000;

    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<ITERATIONS; i++) {
        sniffer.processPacketLegacy(pkt_enc, 100);
        sniffer.processPacketLegacy(pkt_eapol, 100);
        sniffer.processPacketLegacy(pkt_plain, 100);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Legacy Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms, Handshakes: " << sniffer.handshakeCount << std::endl;

    sniffer.handshakeCount = 0;
    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<ITERATIONS; i++) {
        sniffer.processPacketOptimized(pkt_enc, 100);
        sniffer.processPacketOptimized(pkt_eapol, 100);
        sniffer.processPacketOptimized(pkt_plain, 100);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Optimized Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms, Handshakes: " << sniffer.handshakeCount << std::endl;

    return 0;
}
