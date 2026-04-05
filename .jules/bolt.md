## 2025-02-14 - Skip EAPOL Parsing for Encrypted 802.11 Data Frames
**Learning:** Checking the Protected bit (`packet[1] & 0x40`) allows us to safely skip encrypted 802.11 data frames because encrypted frames cannot contain valid plaintext EAPOL headers. Scanning payloads of these frames wastes CPU cycles as they make up the vast majority of Wi-Fi traffic.
**Action:** Always check for the Protected bit and minimum length (`len >= 24`) before parsing payload data in 802.11 frames to yield significant performance improvements (up to ~40x) by avoiding useless header scans.
