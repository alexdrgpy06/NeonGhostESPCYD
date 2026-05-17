## 2026-05-17 - Packet Sniffing Performance Optimization
**Learning:** Encrypted 802.11 data frames cannot contain valid plaintext EAPOL headers. Skipping payload inspection for encrypted frames yields significant performance improvements.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) and perform a minimum length bounds check (e.g., `if (len >= 26)`) before accessing EAPOL header bytes to prevent out-of-bounds access and improve performance when scanning for EAPOL handshakes.
