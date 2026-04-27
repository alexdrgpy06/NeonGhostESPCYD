## 2024-04-27 - Skip encrypted 802.11 frames payload inspection
**Learning:** Encrypted 802.11 data frames (Protected bit 0x40 set in frame control) cannot contain valid plaintext EAPOL headers. Skipping the linear scan for these frames yields significant speedups (~24x in benchmarks).
**Action:** Always check the Protected bit and minimum length bounds before parsing payloads of Data frames.
