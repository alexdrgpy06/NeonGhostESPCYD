## 2025-02-19 - Packet Sniffing Optimization
**Learning:** Encrypted 802.11 data frames (Protected bit set) can be safely skipped when scanning for EAPOL handshakes, as the LLC/SNAP header containing the EtherType is encrypted. This avoids scanning ~90% of traffic.
**Action:** Always check frame control flags before deep packet inspection to filter out irrelevant traffic early.
