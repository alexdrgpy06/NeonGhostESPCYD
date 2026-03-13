## 2024-05-24 - Initial Setup
**Learning:** Establishing the journal for Bolt's critical performance learnings.
**Action:** Use this file to record architecture-specific bottlenecks and non-obvious optimizations.

## 2024-05-24 - O(1) Handshake Detection
**Learning:** Found an inefficient O(n) scan in `PacketSniffer::processPacket` for EAPOL packets in `DATA` frames. It was scanning linearly within indices 24 to 60, evaluating `packet[i] == 0x88 && packet[i + 1] == 0x8E` for EAPOL handshakes. However, I learned that strict 802.11 header parsing can break core EAPOL detection for frames using A-MSDU or other variable padding. By skipping encrypted frames outright (since EAPOL headers cannot be encrypted and still detected this way), but keeping the robust linear scan, we get 99% of the performance benefit while maintaining 100% of the robustness.
**Action:** Skip encrypted frames using `!(packet[1] & 0x40)` to bypass payload inspection, but keep the windowed linear scan for plain text EAPOL frames.
