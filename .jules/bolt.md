## 2024-05-12 - Skipping Encrypted Frames for EAPOL Detection
**Learning:** Scanning all 802.11 data frames for EAPOL headers (0x888E) includes encrypted payloads, wasting CPU cycles on O(N) payload inspection when plaintext EAPOL headers cannot exist there.
**Action:** Always check the 'Protected' bit (`packet[1] & 0x40`) before inspecting the payload of 802.11 data frames to safely bypass encrypted packets and yield a ~25x+ speedup.
