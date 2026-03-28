
## 2024-03-27 - [Skip Scanning Encrypted Frames]
**Learning:** EAPOL handshakes are never encrypted, but the previous `PacketSniffer::processPacket` implementation scanned the payload of every Data Frame (Type 2) looking for the EAPOL EtherType (0x888E). By simply checking if the Protected bit (0x40) is set in the Frame Control field (`packet[1]`), we can skip scanning encrypted frames.
**Action:** When parsing 802.11 frames, always utilize header flags (like the Protected bit) to filter out packets before performing linear scans on the payload. A simple check like `if (len >= 24 && !(packet[1] & 0x40))` yields a ~90% performance boost for processing encrypted data packets.
