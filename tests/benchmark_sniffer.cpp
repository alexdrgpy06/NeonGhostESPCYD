#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <cstdint>

// Global variables to simulate PacketSniffer state
volatile uint32_t handshakeCount = 0;
volatile bool handshakeDetected = false;
volatile bool pendingEvent = false;

// Mock packet processing - ORIGINAL (Linear Scan)
void processPacket_Original(const uint8_t *packet, uint16_t len) {
    if (len < 10) return;

    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;

    // DATA FRAMES (Type 2) - EAPOL Detection
    if (type == 2) {
        for (int i = 24; i < len - 6 && i < 60; i++) {
            if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                handshakeCount++;
                handshakeDetected = true;
                pendingEvent = true;
                break;
            }
        }
    }
}

// Mock packet processing - OPTIMIZED (O(1) Offset Calculation)
void processPacket_Optimized(const uint8_t *packet, uint16_t len) {
    if (len < 10) return;

    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;

    // DATA FRAMES (Type 2) - EAPOL Detection
    if (type == 2) {
        // OPTIMIZATION: Check 'Protected' bit (Bit 6 of Byte 1)
        // If set, payload is encrypted, so we can't see EtherType anyway.
        if (packet[1] & 0x40) return;

        // Calculate offset to LLC/SNAP header
        // Base 802.11 header is 24 bytes
        int offset = 24;

        // Address 4 present? (ToDS and FromDS both set)
        if ((packet[1] & 0x03) == 0x03) {
            offset += 6;
        }

        // QoS Control present? (Subtype bit 7 set in Byte 0)
        // Subtype is bits 4-7 of Byte 0.
        // QoS Data subtypes are 8-15 (0x8 - 0xF), so bit 3 of subtype is set.
        // This corresponds to bit 7 of Byte 0.
        if ((packet[0] & 0x80) == 0x80) { // Check QoS bit
             offset += 2;
        }

        // HT Control present? (Order bit, Bit 7 of Byte 1)
        if (packet[1] & 0x80) {
            offset += 4;
        }

        // Check bounds
        if (offset + 8 > len) return;

        // Check EtherType (Offset + 6 in LLC/SNAP header)
        // LLC/SNAP: AA AA 03 00 00 00 <Type>
        if (packet[offset + 6] == 0x88 && packet[offset + 7] == 0x8E) {
            handshakeCount++;
            handshakeDetected = true;
            pendingEvent = true;
        }
    }
}

void benchmark(const char* name, void (*func)(const uint8_t*, uint16_t), const std::vector<uint8_t>& packet, int iterations) {
    handshakeCount = 0;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func(packet.data(), packet.size());
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;
    std::cout << name << ": " << elapsed.count() / 1000.0 << " ms ("
              << (elapsed.count() / iterations) << " us/op)" << std::endl;
}

int main() {
    std::cout << "PacketSniffer Benchmark\n=======================\n";

    // 1. Encrypted Data (Common case - should be skipped)
    std::vector<uint8_t> pkt_encrypted(100, 0);
    pkt_encrypted[0] = 0x08; // Data, Data
    pkt_encrypted[1] = 0x40; // Protected=1
    // Fill with random "encrypted" data to avoid false positives in scan
    for(int i=24; i<100; i++) pkt_encrypted[i] = rand() % 256;

    // 2. Plaintext EAPOL (Handshake - Rare case)
    std::vector<uint8_t> pkt_eapol(100, 0);
    pkt_eapol[0] = 0x88; // Data, QoS Data (Subtype 8) -> QoS bit set
    pkt_eapol[1] = 0x02; // FromDS=1, Protected=0
    // Header: 24 bytes
    // QoS: 2 bytes (offset 24)
    // LLC starts at 26
    // EtherType at 26 + 6 = 32
    pkt_eapol[32] = 0x88;
    pkt_eapol[33] = 0x8E;

    // 3. Plaintext IP Data (No EAPOL)
    std::vector<uint8_t> pkt_plain(100, 0);
    pkt_plain[0] = 0x08; // Data, Data
    pkt_plain[1] = 0x00; // Protected=0
    // Header: 24 bytes
    // LLC starts at 24
    // EtherType at 30
    pkt_plain[30] = 0x08; // IPv4
    pkt_plain[31] = 0x00;

    int iterations = 10000000;

    std::cout << "\n--- Scenario 1: Encrypted Data (10M iters) ---\n";
    benchmark("Original ", processPacket_Original, pkt_encrypted, iterations);
    benchmark("Optimized", processPacket_Optimized, pkt_encrypted, iterations);

    std::cout << "\n--- Scenario 2: Plaintext EAPOL (10M iters) ---\n";
    benchmark("Original ", processPacket_Original, pkt_eapol, iterations);
    if (handshakeCount != iterations) std::cout << "WARNING: Optimized missed detections! Count=" << handshakeCount << std::endl;
    benchmark("Optimized", processPacket_Optimized, pkt_eapol, iterations);
    if (handshakeCount != iterations) std::cout << "WARNING: Optimized missed detections! Count=" << handshakeCount << std::endl;

    std::cout << "\n--- Scenario 3: Plaintext Data (10M iters) ---\n";
    benchmark("Original ", processPacket_Original, pkt_plain, iterations);
    benchmark("Optimized", processPacket_Optimized, pkt_plain, iterations);

    return 0;
}
