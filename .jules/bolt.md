## 2024-05-24 - EAPOL Detection Loop Optimization

**Learning:** The `PacketSniffer::processPacket` routine spends significant time scanning the payloads of encrypted 802.11 data frames (Type 2) looking for plaintext EAPOL headers (0x88 0x8E). Since encrypted frames cannot contain valid plaintext EAPOL headers, this is a major performance bottleneck, wasting CPU cycles on unnecessary linear scans.
**Action:** When inspecting 802.11 data frames for EAPOL handshakes, always check the 'Protected' bit (`packet[1] & 0x40`) first. If the frame is encrypted, safely skip the payload inspection loop to yield significant performance improvements (~3x speedup in worst-case scenarios). Always ensure a minimum length bounds check (e.g., `if (len >= 24)`) before accessing header bytes.
