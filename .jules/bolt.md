## 2024-04-30 - EAPOL Inspection on Encrypted Frames
**Learning:** Encrypted 802.11 data frames cannot contain valid plaintext EAPOL headers. The previous implementation checked every data frame payload for `0x88 0x8E`, causing significant overhead.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) to safely skip encrypted 802.11 data frames and perform a length bounds check (`len >= 24`) before inspecting the payload for EAPOL headers.
