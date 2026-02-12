## 2024-05-23 - [Packet Sniffer Optimization]
**Learning:** Linear scanning of encrypted WiFi payloads for EAPOL headers (0x88 0x8E) is a major anti-pattern. Checking the 'Protected' bit (0x40 in Frame Control) first allows skipping ~80% of frames (encrypted data).
**Action:** Always check the 'Protected' bit before parsing payload content in 802.11 sniffers. Use O(1) offset calculation based on header flags (QoS, HT, 4-Addr) instead of scanning.
