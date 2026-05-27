## 2024-05-28 - Avoid String allocations in hot paths
**Learning:** Found String allocations in ISR / hot paths like `PacketSniffer::processPacket` and `SDManager::getNextFileName`. The memory usage and allocation time overhead is huge for ESP32.
**Action:** Replace String operations with char buffers (`snprintf`) and pass `const char*` directly where possible.

## 2026-05-27 - Skipping Encrypted Frames for EAPOL Detection
**Learning:** Checking the 'Protected' bit (packet[1] & 0x40) on 802.11 data frames allows safely skipping payload inspection for encrypted frames, yielding a ~22x to 47x speedup since encrypted frames can't contain valid plaintext EAPOL headers.
**Action:** Always check the 'Protected' bit before scanning data frames for EAPOL headers, ensuring a bounds check (len >= 26) is done first.
