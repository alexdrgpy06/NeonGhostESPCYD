## 2024-05-24 - [Skip Encrypted 802.11 Data Frames]
**Learning:** Checking the 'Protected' bit (`packet[1] & 0x40`) inside `PacketSniffer::processPacket` prevents expensive payload inspection on encrypted 802.11 data frames which cannot contain valid plaintext EAPOL headers.
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 24)`) before accessing header bytes to avoid undefined behavior, and skip parsing if encrypted for a significant performance gain.
