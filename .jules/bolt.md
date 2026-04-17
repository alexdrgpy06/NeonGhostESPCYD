## 2024-11-13 - [Fast EAPOL Scan in WPA]
**Learning:** The `PacketSniffer::processPacket` routine is doing a linear byte scan for WPA EAPOL packets on all Type 2 (data) packets. WPA EAPOL exchanges occur as plaintext data packets, whereas regular Type 2 data packets are encrypted (Protected bit is set, `packet[1] & 0x40`). Scanning an encrypted packet is a waste of CPU.
**Action:** Always verify `len >= 24` bounds check and `!(packet[1] & 0x40)` when searching for WPA handshakes in `processPacket` to drastically optimize data frame evaluation.
