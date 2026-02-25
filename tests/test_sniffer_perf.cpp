#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdint>

// Bolt: Benchmark for EAPOL Packet Sniffing Performance
// This test compares the original linear search implementation vs the O(1) optimized implementation.
// Run with: g++ -O2 tests/test_sniffer_perf.cpp -o benchmark && ./benchmark

// Mock PacketSniffer class structure for benchmarking
struct PacketSniffer {
    volatile uint32_t packetCount = 0;
    volatile uint32_t handshakeCount = 0;
    volatile bool handshakeDetected = false;

    // Original implementation (simulating src/PacketSniffer.cpp)
    // Linear scan looking for 0x88 0x8E
    void processPacket_Original(uint8_t *packet, uint16_t len) {
        packetCount++;
        if (len < 10) return;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;

        // DATA FRAMES (Type 2) - EAPOL Detection
        if (type == 2) {
            for (int i = 24; i < len - 6 && i < 60; i++) {
                if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                    handshakeCount++;
                    handshakeDetected = true;
                    break;
                }
            }
        }
    }

    // Optimized implementation (O(1) offset calculation)
    void processPacket_Optimized(uint8_t *packet, uint16_t len) {
        packetCount++;

        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;

        // DATA FRAMES (Type 2) only
        if (type != 2) return;

        // Skip encrypted frames (Protected bit is bit 6 of byte 1)
        if (packet[1] & 0x40) return;

        // Calculate header length
        // Base header: 24 bytes
        int offset = 24;

        // Address 4 field present? (ToDS=1 and FromDS=1)
        // ToDS is bit 0 of byte 1, FromDS is bit 1 of byte 1
        if ((packet[1] & 0x03) == 0x03) {
            offset += 6;
        }

        // QoS Control field present? (Subtype bit 7 is 1)
        // Subtype is bits 4-7 of byte 0.
        uint8_t subtype = (frameControl >> 4) & 0x0F;
        if (subtype & 0x08) {
            offset += 2;
        }

        // HT Control field present? (Order bit is 1)
        // Order is bit 7 of byte 1.
        if (packet[1] & 0x80) {
            offset += 4;
        }

        // Check bounds (Offset + LLC/SNAP header length 8)
        if (len < offset + 8) return;

        // Check for LLC/SNAP header (8 bytes)
        // DSAP (1) + SSAP (1) + Control (1) + OUI (3) + Type (2)
        // We look for Type = 0x888E at offset + 6
        if (packet[offset + 6] == 0x88 && packet[offset + 7] == 0x8E) {
            handshakeCount++;
            handshakeDetected = true;
        }
    }
};

int main() {
    PacketSniffer sniffer;
    const int ITERATIONS = 10000000;

    // Test Case 1: Encrypted Data Packet (should be skipped fast)
    // Type 2, Protected bit set (0x40 in byte 1)
    uint8_t encryptedPacket[100];
    memset(encryptedPacket, 0, 100);
    encryptedPacket[0] = 0x08; // Type 2 (Data), Subtype 0
    encryptedPacket[1] = 0x40; // Protected bit set

    // Test Case 2: EAPOL Packet (should be detected)
    // Type 2, Not Protected, Subtype 0 (Data)
    // Header length 24.
    // LLC/SNAP at 24: AA AA 03 00 00 00 88 8E
    uint8_t eapolPacket[100];
    memset(eapolPacket, 0, 100);
    eapolPacket[0] = 0x08; // Type 2 (Data), Subtype 0
    eapolPacket[1] = 0x00; // Not protected
    // Fill LLC/SNAP at offset 24
    eapolPacket[24] = 0xAA;
    eapolPacket[25] = 0xAA;
    eapolPacket[26] = 0x03;
    eapolPacket[27] = 0x00;
    eapolPacket[28] = 0x00;
    eapolPacket[29] = 0x00;
    eapolPacket[30] = 0x88; // EtherType MSB
    eapolPacket[31] = 0x8E; // EtherType LSB

    // Test Case 3: QoS EAPOL Packet
    // Type 2, Subtype 8 (QoS Data) -> Header +2 bytes
    uint8_t qosEapolPacket[100];
    memset(qosEapolPacket, 0, 100);
    qosEapolPacket[0] = 0x88; // Type 2 (Data), Subtype 8 (QoS)
    qosEapolPacket[1] = 0x00; // Not protected
    // Offset is 24 + 2 (QoS) = 26
    qosEapolPacket[26] = 0xAA;
    qosEapolPacket[27] = 0xAA;
    qosEapolPacket[28] = 0x03;
    qosEapolPacket[29] = 0x00;
    qosEapolPacket[30] = 0x00;
    qosEapolPacket[31] = 0x00;
    qosEapolPacket[32] = 0x88;
    qosEapolPacket[33] = 0x8E;

    std::cout << "Benchmarking PacketSniffer::processPacket..." << std::endl;
    std::cout << "Iterations: " << ITERATIONS << std::endl;

    // --- Benchmark Original ---
    sniffer.packetCount = 0;
    sniffer.handshakeCount = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; i++) {
        sniffer.processPacket_Original(encryptedPacket, 100);
        sniffer.processPacket_Original(eapolPacket, 100);
        sniffer.processPacket_Original(qosEapolPacket, 100);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto durationOriginal = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Original Implementation: " << durationOriginal << " ms" << std::endl;
    std::cout << "Handshakes detected: " << sniffer.handshakeCount << std::endl;

    // --- Benchmark Optimized ---
    sniffer.packetCount = 0;
    sniffer.handshakeCount = 0;
    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; i++) {
        sniffer.processPacket_Optimized(encryptedPacket, 100);
        sniffer.processPacket_Optimized(eapolPacket, 100);
        sniffer.processPacket_Optimized(qosEapolPacket, 100);
    }

    end = std::chrono::high_resolution_clock::now();
    auto durationOptimized = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Optimized Implementation: " << durationOptimized << " ms" << std::endl;
    std::cout << "Handshakes detected: " << sniffer.handshakeCount << std::endl;

    double improvement = (double)durationOriginal / durationOptimized;
    std::cout << "Speedup: " << improvement << "x" << std::endl;

    return 0;
}
