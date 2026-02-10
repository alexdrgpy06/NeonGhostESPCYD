## 2024-05-22 - ISR Performance in PacketSniffer
**Learning:** The `PacketSniffer::processPacket` function runs in the promiscuous mode callback (ISR context) for *every* WiFi packet. The original implementation used an O(N) loop to scan for EAPOL headers, which wastes CPU cycles and risks WDT resets during high traffic.
**Action:** Always prefer O(1) deterministic header parsing over linear scanning in high-frequency packet processing paths. Calculating offsets based on frame control flags is both faster and more correct (fewer false positives).
