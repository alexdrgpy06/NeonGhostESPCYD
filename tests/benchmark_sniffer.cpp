#include <iostream>
#include <vector>
#include <stdint.h>
#include <chrono>

using namespace std;

// Old linear scan
bool old_scan(const uint8_t* packet, uint16_t len) {
    for (int i = 24; i < len - 6 && i < 60; i++) {
        if (packet[i] == 0x88 && packet[i + 1] == 0x8E) {
            return true;
        }
    }
    return false;
}

// New O(1) scan
bool new_scan(const uint8_t* packet, uint16_t len) {
    if (len < 10) return false;
    uint8_t frameControl0 = packet[0];
    uint8_t frameControl1 = packet[1];
    uint8_t subtype = (frameControl0 >> 4) & 0x0F;

    // Check Protected bit (0x40 in FC byte 1)
    if (frameControl1 & 0x40) {
        return false; // Encrypted
    }

    uint16_t headerLen = 24;

    // Address 4 (ToDS = 1, FromDS = 1) -> FC byte 1 bits 0 and 1
    if ((frameControl1 & 0x03) == 0x03) {
        headerLen += 6;
    }

    // QoS Control (subtype bit 3 is set -> QoS Data)
    bool isQoS = (subtype & 0x08) != 0;
    if (isQoS) {
        headerLen += 2;
        // HT Control (Order bit in FC byte 1 is set AND it's a QoS Data frame)
        if (frameControl1 & 0x80) {
            headerLen += 4;
        }
    }

    // LLC starts at headerLen
    // EAPOL type is at offset 6 within LLC
    uint16_t eapolOffset = headerLen + 6;

    if (eapolOffset + 1 < len) {
        if (packet[eapolOffset] == 0x88 && packet[eapolOffset + 1] == 0x8E) {
            return true;
        }
    }
    return false;
}

int main() {
    // Dummy EAPOL packet without QoS
    uint8_t pkt1[60] = {0};
    pkt1[0] = 0x08; // Data
    pkt1[1] = 0x00; // No ToDS/FromDS, not protected, not order
    pkt1[24+0] = 0xAA;
    pkt1[24+1] = 0xAA;
    pkt1[24+2] = 0x03;
    pkt1[24+3] = 0x00;
    pkt1[24+4] = 0x00;
    pkt1[24+5] = 0x00;
    pkt1[24+6] = 0x88;
    pkt1[24+7] = 0x8E;

    cout << "Pkt1 Old: " << old_scan(pkt1, 60) << " New: " << new_scan(pkt1, 60) << endl;

    // Dummy QoS EAPOL packet
    uint8_t pkt2[60] = {0};
    pkt2[0] = 0x88; // QoS Data (Subtype 8)
    pkt2[1] = 0x00;
    pkt2[26+6] = 0x88;
    pkt2[26+7] = 0x8E;

    cout << "Pkt2 Old: " << old_scan(pkt2, 60) << " New: " << new_scan(pkt2, 60) << endl;

    // Encrypted QoS packet
    uint8_t pkt3[60] = {0};
    pkt3[0] = 0x88; // QoS Data
    pkt3[1] = 0x40; // Protected
    pkt3[26+6] = 0x88; // Dummy matching signature
    pkt3[26+7] = 0x8E;

    cout << "Pkt3 Old: " << old_scan(pkt3, 60) << " New: " << new_scan(pkt3, 60) << endl;

    // Benchmark
    auto start = chrono::high_resolution_clock::now();
    int count = 0;
    for(int i=0; i<1000000; i++) {
        if(old_scan(pkt1, 60)) count++;
        if(old_scan(pkt2, 60)) count++;
        if(old_scan(pkt3, 60)) count++;
    }
    auto end = chrono::high_resolution_clock::now();
    cout << "Old scan took: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " us" << endl;

    start = chrono::high_resolution_clock::now();
    count = 0;
    for(int i=0; i<1000000; i++) {
        if(new_scan(pkt1, 60)) count++;
        if(new_scan(pkt2, 60)) count++;
        if(new_scan(pkt3, 60)) count++;
    }
    end = chrono::high_resolution_clock::now();
    cout << "New scan took: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << " us" << endl;

    return 0;
}
