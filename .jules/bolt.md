## 2024-05-04 - Skip Payload Inspection for Encrypted 802.11 Data Frames
**Learning:** Encrypted frames cannot contain plaintext EAPOL headers. Skipping the linear payload scan for frames with the Protected bit set (`packet[1] & 0x40`) yields significant performance improvements (~40-47x speedup).
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 24)`) and check the Protected bit before accessing header bytes in data frames.
