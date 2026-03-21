## 2024-05-24 - [Skip scanning encrypted WiFi data frames]
**Learning:** [Linear scanning of all WiFi data frames for EAPOL handshakes is a performance bottleneck. Checking the Protected bit (`packet[1] & 0x40`) skips encrypted frames, yielding a ~25x speedup.]
**Action:** [When processing WiFi data frames, always check the Protected bit and minimum length before scanning for handshakes to avoid unnecessary CPU cycles.]
