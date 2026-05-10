## 2024-05-24 - Skip Encrypted Data Frames
**Learning:** Checking the 'Protected' bit (`packet[1] & 0x40`) in 802.11 data frames safely skips encrypted frames that cannot contain plaintext EAPOL headers, avoiding useless payload iteration and yielding massive performance improvements.
**Action:** Always ensure a minimum length bounds check (`if (len >= 24)`) and check the Protected bit before scanning frame payloads for specific plaintext signatures.
