## 2024-05-24 - Avoid scanning encrypted 802.11 payloads
**Learning:** Encrypted 802.11 frames (where the Protected bit `packet[1] & 0x40` is set) cannot contain unencrypted EAPOL headers. Skipping the linear scan over these frames yields a ~21.8x performance improvement (from ~1090ms to ~50ms).
**Action:** Always check the Protected bit and minimum length (`len >= 24`) before parsing payload data in Data (Type 2) frames to avoid unnecessary computation.
