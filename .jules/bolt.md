## 2024-05-24 - Skip Encrypted Frames Payload Inspection
**Learning:** The `PacketSniffer::processPacket` routine was unnecessarily inspecting the payload of all data frames for EAPOL headers. Encrypted frames (where the 'Protected' bit `packet[1] & 0x40` is set) cannot contain valid plaintext EAPOL headers.
**Action:** Always ensure a minimum length bounds check (`if (len >= 24)`) and check the 'Protected' bit (`!(packet[1] & 0x40)`) before accessing header bytes in 802.11 data frames to skip payload inspection and improve performance.
