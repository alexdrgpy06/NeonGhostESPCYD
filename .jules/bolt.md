## 2026-05-25 - Optimized PacketSniffer Payload Inspection
**Learning:** Encrypted 802.11 data frames cannot contain valid plaintext EAPOL headers. The previous implementation checked the payload for every data frame, causing a severe performance bottleneck during heavy traffic.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) and ensure a minimum length bounds check (e.g., `if (len >= 26)`) before accessing EAPOL header bytes to prevent out-of-bounds access and bypass unnecessary inspection.
