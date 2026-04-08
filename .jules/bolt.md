## 2024-05-24 - Optimization: Skip Encrypted Data Frames during EAPOL Scan
**Learning:** During packet processing, running a linear EAPOL scan (looking for `0x888E`) over the payload of all data frames is inefficient when the data is encrypted. Encrypted 802.11 data frames cannot contain a valid plaintext EAPOL header. Processing these frames wastes CPU cycles (approx. 23x speedup when skipped).
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) before performing payload inspection on data frames to safely bypass encrypted payloads.
