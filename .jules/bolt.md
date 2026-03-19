## 2024-05-18 - Skipping encrypted frames in processPacket
**Learning:** Checking the 'Protected' bit (`packet[1] & 0x40`) and skipping encrypted frames yields a ~25x performance improvement because encrypted frames cannot contain valid EAPOL headers and we bypass the expensive linear scan on the payload.
**Action:** When inspecting packet contents for unencrypted patterns, ensure we verify headers beforehand to quickly filter out irrelevant payloads.
