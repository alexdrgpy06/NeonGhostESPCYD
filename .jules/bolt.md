## 2025-02-27 - [WiFi Sniffer Optimization]
**Learning:** In 802.11 promiscuous mode, blindly scanning packet payloads for cleartext EAPOL headers (0x888E) consumes significant CPU cycles because most modern traffic is encrypted (Protected bit set).
**Action:** Always check the Frame Control "Protected" bit (`packet[1] & 0x40`) before deep packet inspection. EAPOL frames (which we care about for handshakes) are data frames but are NOT encrypted at the 802.11 link layer (Protected bit is 0), so we can safely skip any packet with this bit set. This avoids processing >90% of traffic. Also, avoid `String` in packet callbacks to prevent heap fragmentation.
## 2025-02-27 - [EAPOL Parsing Optimization]
**Learning:** Scanning packet buffers linearly for EAPOL signatures (0x888E) creates false positives when the sequence appears in MAC addresses (e.g., Address 4).
**Action:** Always parse the 802.11 header to calculate the exact offset of the LLC/SNAP payload. Be sure to account for Address 4 (ToDS/FromDS), QoS Control (Subtype), and HT Control (Order bit) fields to locate the EtherType reliably. This transforms the operation from O(N) to O(1) and ensures correctness.
