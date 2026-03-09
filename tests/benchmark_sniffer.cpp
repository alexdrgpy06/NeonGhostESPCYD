#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

int processPacketLegacy(uint8_t *packet, uint16_t len) {
    if (len < 10) return 0;

    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;
    uint8_t subtype = (frameControl >> 4) & 0x0F;

    int count = 0;
    if (type == 2) {
        for (int i = 24; i < len - 6 && i < 60; i++) {
            if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
                count++;
                break;
            }
        }
    }
    return count;
}

int processPacketOptimized(uint8_t *packet, uint16_t len) {
    if (len < 10) return 0;

    uint8_t frameControl = packet[0];
    uint8_t type = (frameControl >> 2) & 0x03;
    uint8_t subtype = (frameControl >> 4) & 0x0F;

    int count = 0;
    if (type == 2) {
        // Skip encrypted frames
        if (!(packet[1] & 0x40)) {
            uint16_t headerLen = 24;
            if ((packet[1] & 0x03) == 0x03) headerLen += 6;
            if (subtype & 0x08) {
                headerLen += 2;
                if (packet[1] & 0x80) headerLen += 4;
            }

            if (headerLen + 7 < len) {
                if (packet[headerLen + 6] == 0x88 && packet[headerLen + 7] == 0x8E) {
                    count++;
                }
            }
        }
    }
    return count;
}

int main() {
    // Generate some packets
    vector<vector<uint8_t>> packets;
    for (int i = 0; i < 10000; i++) {
        vector<uint8_t> pkt(100, 0);
        pkt[0] = 0x08; // Type 2 (Data), Subtype 0 (Data)
        pkt[1] = 0x00; // Unencrypted
        if (i % 10 == 0) {
            // EAPOL
            pkt[24+6] = 0x88;
            pkt[24+7] = 0x8E;
        } else if (i % 10 == 1) {
            // Encrypted
            pkt[1] = 0x40;
        }
        packets.push_back(pkt);
    }

    auto start = chrono::high_resolution_clock::now();
    int count1 = 0;
    for (int k = 0; k < 1000; k++) {
        for (auto& pkt : packets) {
            count1 += processPacketLegacy(pkt.data(), pkt.size());
        }
    }
    auto end = chrono::high_resolution_clock::now();
    cout << "Legacy count: " << count1 << " Time: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " us\n";

    start = chrono::high_resolution_clock::now();
    int count2 = 0;
    for (int k = 0; k < 1000; k++) {
        for (auto& pkt : packets) {
            count2 += processPacketOptimized(pkt.data(), pkt.size());
        }
    }
    end = chrono::high_resolution_clock::now();
    cout << "Optimized count: " << count2 << " Time: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " us\n";

    return 0;
}
