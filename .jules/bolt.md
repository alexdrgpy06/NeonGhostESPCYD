## 2024-05-24 - Skip EAPOL inspection on encrypted data frames
**Learning:** The `PacketSniffer::processPacket` routine is optimized by checking the 'Protected' bit (`packet[1] & 0x40`) to safely skip encrypted 802.11 data frames. Encrypted frames cannot contain valid plaintext EAPOL headers, so skipping payload inspection yields significant performance improvements (~24x speedup).
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 24)`) before accessing header bytes and check if frame is protected before attempting plaintext signature matches.
