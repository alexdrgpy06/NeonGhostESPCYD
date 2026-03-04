#include <iostream>
#include <vector>
#include <chrono>

// Mock Data Structure
struct NetworkInfo {
    uint8_t bssid[6];
    char ssid[33];
    int8_t rssi;
    uint8_t channel;
    bool hasHandshake;
};

// Mock the Sniffer state and necessary globals
int handshakeCount = 0;
bool handshakeDetected = false;
bool savePacket = false;
NetworkInfo networks[10];

int findNetwork(uint8_t* bssid) {
    return 0; // Always return 0 for benchmark
}

// Function 1: Legacy Linear Scan
void processPacketLegacy(uint8_t *packet, uint16_t len) {
    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;

    if (type == 2) {
        for (int i = 24; i < len - 6 && i < 60; i++) {
            if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                handshakeCount++;
                handshakeDetected = true;
                savePacket = true;
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

// Function 2: Optimized O(1) Header Parsing
void processPacketOptimized(uint8_t *packet, uint16_t len) {
    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;
    uint8_t subtype = (frameControl >> 4) & 0x0F;

    if (type == 2) {
        if ((packet[1] & 0x40) == 0) {
            int headerLen = 24;
            uint8_t toDS = packet[1] & 0x01;
            uint8_t fromDS = (packet[1] >> 1) & 0x01;

            if (toDS && fromDS) {
                headerLen += 6;
            }

            if (subtype & 0x08) {
                headerLen += 2;
                if (packet[1] & 0x80) {
                    headerLen += 4;
                }
            }

            int llcLen = 6;
            int offset = headerLen + llcLen;

            if (offset + 1 < len) {
                if (packet[offset] == 0x88 && packet[offset + 1] == 0x8E) {
                    handshakeCount++;
                    handshakeDetected = true;
                    savePacket = true;
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

int main() {
    const int ITERATIONS = 10000000;

    // Mock Plaintext EAPOL Packet (Type 2, QoS Data, ToDS set)
    std::vector<uint8_t> plainPacket(80, 0);
    plainPacket[0] = 0x08; // Frame Control byte 0: Type 2, Subtype 0 (Data)
    plainPacket[1] = 0x01; // Frame Control byte 1: ToDS=1, Protected=0
    plainPacket[30] = 0x88; // 24 (Base) + 6 (LLC) = 30
    plainPacket[31] = 0x8E;

    // Mock Encrypted Data Packet
    std::vector<uint8_t> encPacket(80, 0);
    encPacket[0] = 0x08; // Frame Control byte 0: Type 2
    encPacket[1] = 0x41; // Frame Control byte 1: ToDS=1, Protected=1 (0x40)

    // ==========================================
    // Benchmark 1: Plaintext EAPOL Packet
    // ==========================================
    std::cout << "--- Benchmarking Plaintext Packet ---" << std::endl;

    // Legacy
    auto startLegacyPlain = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        processPacketLegacy(plainPacket.data(), plainPacket.size());
    }
    auto endLegacyPlain = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationLegacyPlain = endLegacyPlain - startLegacyPlain;
    std::cout << "Legacy Time:    " << durationLegacyPlain.count() << " ms\n";

    // Optimized
    auto startOptPlain = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        processPacketOptimized(plainPacket.data(), plainPacket.size());
    }
    auto endOptPlain = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationOptPlain = endOptPlain - startOptPlain;
    std::cout << "Optimized Time: " << durationOptPlain.count() << " ms\n";
    std::cout << "Improvement:    " << durationLegacyPlain.count() / durationOptPlain.count() << "x\n\n";

    // ==========================================
    // Benchmark 2: Encrypted Data Packet
    // ==========================================
    std::cout << "--- Benchmarking Encrypted Packet ---" << std::endl;

    // Legacy
    auto startLegacyEnc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        processPacketLegacy(encPacket.data(), encPacket.size());
    }
    auto endLegacyEnc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationLegacyEnc = endLegacyEnc - startLegacyEnc;
    std::cout << "Legacy Time:    " << durationLegacyEnc.count() << " ms\n";

    // Optimized
    auto startOptEnc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        processPacketOptimized(encPacket.data(), encPacket.size());
    }
    auto endOptEnc = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationOptEnc = endOptEnc - startOptEnc;
    std::cout << "Optimized Time: " << durationOptEnc.count() << " ms\n";
    std::cout << "Improvement:    " << durationLegacyEnc.count() / durationOptEnc.count() << "x\n";

    return 0;
}
