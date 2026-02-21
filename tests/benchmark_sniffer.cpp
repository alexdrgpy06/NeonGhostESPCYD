#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdint>

// Mock SDManager
class SDManager {
public:
    void addPacket(uint8_t* packet, uint16_t len) {}
};

// Mock PacketSniffer state
class PacketSniffer {
public:
    SDManager* sdManager = nullptr;
    volatile uint32_t packetCount = 0;
    volatile uint32_t handshakeCount = 0;
    volatile bool handshakeDetected = false;
    volatile int pendingEvent = 0;
    std::string eventDetails;

    // Original implementation (copied from src/PacketSniffer.cpp)
    void processPacket_Original(uint8_t *packet, uint16_t len) {
        packetCount++;
        if (len < 10) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;
        // uint8_t subtype = (frameControl >> 4) & 0x0F; // Unused in this test case for Type 2

        // DATA FRAMES (Type 2) - EAPOL Detection
        if (type == 2) {
            for (int i = 24; i < len - 6 && i < 60; i++) {
                if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                    handshakeCount++;
                    handshakeDetected = true;
                    pendingEvent = 1; // EVT_HANDSHAKE
                    eventDetails = "WPA HANDSHAKE";

                    // Mark network as having handshake (Simulated)
                    // uint8_t* bssid = &packet[16];
                    // ... lookup logic omitted ...
                    break;
                }
            }
        }
    }

    // Optimized implementation
    void processPacket_Optimized(uint8_t *packet, uint16_t len) {
        packetCount++;
        if (len < 10) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;

        // DATA FRAMES (Type 2) - EAPOL Detection
        if (type == 2) {
            // Check Protected bit (Bit 6 of Frame Control 2nd byte, which is packet[1])
            // 0x40 = 0100 0000
            if (packet[1] & 0x40) return; // Skip encrypted frames

            // Calculate header length
            int headerLen = 24;

            // ToDS & FromDS (Bits 0 & 1 of packet[1])
            if ((packet[1] & 0x03) == 0x03) headerLen += 6;

            // QoS (Subtype bit 3 is set)
            // Subtype is bits 4-7 of packet[0].
            // (packet[0] & 0xF0) >> 4 gives subtype.
            // Check bit 3 of subtype: 0x8.
            if ((packet[0] & 0x80) == 0x80) headerLen += 2; // (packet[0] & 0xF0) & 0x80 is simpler check for bit 7 of byte 0

            // Order (Bit 7 of packet[1])
            if (packet[1] & 0x80) headerLen += 4;

            // EAPOL is 0x888E. LLC header is 8 bytes.
            // LLC/SNAP: AA AA 03 00 00 00 [Type]
            // Type is at offset 6 in LLC header.
            int typeOffset = headerLen + 6;

            if (typeOffset + 1 < len) {
                if (packet[typeOffset] == 0x88 && packet[typeOffset + 1] == 0x8E) {
                    handshakeCount++;
                    handshakeDetected = true;
                    pendingEvent = 1; // EVT_HANDSHAKE
                    eventDetails = "WPA HANDSHAKE";
                }
            }
        }
    }
};

int main() {
    PacketSniffer sniffer;

    // Create a sample EAPOL packet
    // Header: 24 bytes
    // QoS: No
    // Address 4: No
    // LLC: 8 bytes (AA AA 03 00 00 00 88 8E)
    // Payload: ...
    uint8_t packet[100];
    memset(packet, 0, 100);

    // Frame Control: Type 2 (Data), Subtype 0 (Data)
    packet[0] = 0x08; // Type: 10 (Data), Subtype: 0000
    packet[1] = 0x00; // Flags: 0

    // Fill header with dummy data
    for(int i=2; i<24; i++) packet[i] = 0xAA;

    // LLC Header
    packet[24] = 0xAA;
    packet[25] = 0xAA;
    packet[26] = 0x03;
    packet[27] = 0x00;
    packet[28] = 0x00;
    packet[29] = 0x00;
    packet[30] = 0x88; // EtherType High
    packet[31] = 0x8E; // EtherType Low

    // Benchmark
    const int iterations = 10000000;

    auto start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<iterations; i++) {
        sniffer.processPacket_Original(packet, 100);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Original: " << diff.count() << " s" << std::endl;

    sniffer.handshakeCount = 0;

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<iterations; i++) {
        sniffer.processPacket_Optimized(packet, 100);
    }
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "Optimized: " << diff.count() << " s" << std::endl;

    // Test with "Protected" bit set (should be skipped)
    packet[1] |= 0x40;

    start = std::chrono::high_resolution_clock::now();
    for(int i=0; i<iterations; i++) {
        sniffer.processPacket_Optimized(packet, 100);
    }
    end = std::chrono::high_resolution_clock::now();
    diff = end - start;
    std::cout << "Optimized (Encrypted/Skipped): " << diff.count() << " s" << std::endl;

    return 0;
}
