## 2026-05-18 - Optimize EAPOL scanning by skipping encrypted 802.11 data frames
**Learning:** Encrypted frames cannot contain valid plaintext EAPOL headers, so checking the 'Protected' bit (`packet[1] & 0x40`) before payload inspection safely skips them.
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 26)`) and skip payload inspection for encrypted frames when scanning for EAPOL headers.
