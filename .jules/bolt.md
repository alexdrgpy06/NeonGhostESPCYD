## 2024-05-15 - Fast-Pathing Packet Processing
**Learning:** Encrypted 802.11 data frames (Protected bit 0x40 set in frame control) cannot contain valid plaintext EAPOL headers. Skipping the linear scan for the EAPOL EtherType on encrypted packets yields a ~24x speedup.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) and bounds (`len >= 24`) before initiating deep payload inspection on 802.11 frames.
