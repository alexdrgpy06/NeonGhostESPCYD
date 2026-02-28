#include <iostream>
#include <vector>
#include <stdint.h>
#include <chrono>

int findNetwork(uint8_t* bssid) { return -1; }

int main() {
    uint8_t packet[128] = {0};
    uint16_t len = 128;

    // Simulate a data frame
    packet[0] = 0x08; // Type 2 (Data), Subtype 0 (Data)
    packet[1] = 0x00; // No ToDS, FromDS, Protected, Order
    packet[24+6] = 0x88;
    packet[24+7] = 0x8E;

    // Legacy logic
    bool handshake1 = false;
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 1000000; iter++) {
        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;
        if (type == 2) {
            for (int i = 24; i < len - 6 && i < 60; i++) {
                if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                    handshake1 = true;
                    break;
                }
            }
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();

    // Optimized logic
    bool handshake2 = false;
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 1000000; iter++) {
        uint8_t frameControl = packet[0];
        uint8_t type = (frameControl >> 2) & 0x03;
        uint8_t subtype = (frameControl >> 4) & 0x0F;
        if (type == 2) {
            if (!(packet[1] & 0x40)) { // Not protected
                uint16_t headerLen = 24;
                bool toDS = packet[1] & 0x01;
                bool fromDS = packet[1] & 0x02;
                if (toDS && fromDS) headerLen += 6;
                if (subtype & 0x08) {
                    headerLen += 2;
                    if (packet[1] & 0x80) headerLen += 4;
                }

                if (len >= headerLen + 8) {
                    if (packet[headerLen + 6] == 0x88 && packet[headerLen + 7] == 0x8E) {
                        handshake2 = true;
                    }
                }
            }
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();

    std::cout << "Legacy: " << std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() << " us, result: " << handshake1 << std::endl;
    std::cout << "Optimized: " << std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count() << " us, result: " << handshake2 << std::endl;
    return 0;
}
