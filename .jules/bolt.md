## 2026-05-23 - Skip EAPOL scan for encrypted frames
**Learning:** Encrypted data frames make up the vast majority of WiFi traffic, but cannot contain valid plaintext EAPOL headers. The previous O(N) loop uselessly scanned their payloads, wasting CPU cycles in the high-frequency packet processing interrupt/task path. Checking the 'Protected' bit (`packet[1] & 0x40`) safely skips this payload inspection.
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 26)`) before accessing EAPOL header bytes to prevent out-of-bounds access.
