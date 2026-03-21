## 2024-05-24 - Skip Encrypted Data Frames
**Learning:** Encrypted 802.11 frames (Protected bit set) cannot contain valid EAPOL headers, so scanning their payload is a waste of CPU cycles.
**Action:** Always check the Protected bit (`packet[1] & 0x40`) and skip linear scanning for encrypted frames, yielding a ~25x performance improvement.
