## 2024-05-28 - Optimizing Packet Parsing by Skipping Encrypted Frames
**Learning:** Checking the Protected bit (`packet[1] & 0x40`) in 802.11 frames allows us to skip payload inspection for encrypted packets (~90% speedup) because encrypted frames cannot contain valid plaintext EAPOL headers.
**Action:** Always check for the Protected bit and ensure minimum length bounds (`len >= 24`) before inspecting payloads to prevent out-of-bounds memory access and save processing time on encrypted data.
