## 2024-05-24 - [Protected Bit Optimization]
**Learning:** Checking the Protected bit (`packet[1] & 0x40`) before scanning payload for encrypted 802.11 frames prevents searching for EAPOL headers when they wouldn't exist unencrypted, improving processing time drastically.
**Action:** Always check the Protected bit and ensure minimum length bounds (`len >= 24`) before parsing frame headers on data packets to skip unnecessary payload scans.
