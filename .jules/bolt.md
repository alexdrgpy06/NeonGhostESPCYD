
## 2024-05-24 - [O(1) Packet Processing]
**Learning:** The legacy linear scan for EAPOL handshakes in `PacketSniffer::processPacket` is a bottleneck. By calculating the exact start of the payload using an O(1) offset calculation and immediately skipping encrypted frames, we observed a ~2.4x speedup for plaintext handshakes (529ms -> 219ms) and ~3.6x speedup for encrypted data frames (445ms -> 122ms).
**Action:** When parsing standard network protocols with variable-length headers, use deterministic header size calculations instead of linearly scanning for magic bytes whenever possible.
