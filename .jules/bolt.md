## 2024-04-03 - [Packet Sniffer Optimization]
**Learning:** The `PacketSniffer::processPacket` routine can be optimized by checking the 'Protected' bit (`packet[1] & 0x40`) to safely skip encrypted 802.11 data frames. Encrypted frames cannot contain valid plaintext EAPOL headers, so skipping payload inspection yields significant performance improvements (up to ~40x).
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 24)`) before accessing header bytes.
