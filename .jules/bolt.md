## 2024-05-24 - Skip payload inspection on encrypted frames
**Learning:** Checking the 'Protected' bit (`packet[1] & 0x40`) in 802.11 frame control header allows us to safely skip encrypted data frames. Encrypted frames cannot contain valid plaintext EAPOL headers, so skipping payload inspection yields significant performance improvements.
**Action:** Always ensure a minimum length bounds check (e.g., `if (len >= 24)`) before accessing header bytes and checking the encrypted flag to avoid out-of-bounds access.
