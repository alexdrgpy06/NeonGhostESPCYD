## 2024-05-24 - Skip Encrypted Frames in EAPOL Scanning
**Learning:** The `PacketSniffer::processPacket` routine spent significant time searching the payloads of encrypted 802.11 data frames for plaintext EAPOL headers. Since encrypted frames cannot contain valid plaintext EAPOL headers, this scanning was unnecessary.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) and safely skip encrypted 802.11 data frames to avoid wasted CPU cycles. Also ensure proper minimum length bounds checks (`if (len >= 26)`) are in place before accessing header bytes.
