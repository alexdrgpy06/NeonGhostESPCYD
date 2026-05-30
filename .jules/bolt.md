## 2026-05-29 - Optimize EAPOL scanning by skipping encrypted frames
**Learning:** Encrypted 802.11 data frames cannot contain valid plaintext EAPOL headers. Scanning them for the 0x888E signature is a waste of CPU cycles and causes a performance bottleneck during promiscuous packet sniffing.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) in the MAC header before inspecting payloads, and verify the minimum packet length (e.g., `len >= 26`) to prevent out-of-bounds access.
