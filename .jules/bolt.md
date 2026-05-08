## 2024-05-24 - Skip EAPOL inspection on encrypted frames
**Learning:** Encrypted 802.11 data frames (Protected bit 0x40 set in Frame Control) cannot contain valid plaintext EAPOL headers. Scanning their payloads wastes CPU cycles and causes potential false positives.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) and skip payload inspection for encrypted data frames.
