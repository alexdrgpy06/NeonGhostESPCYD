## 2025-05-18 - [Optimizing EAPOL Detection]
**Learning:** Checking the `Protected` bit (bit 6 of Frame Control byte 1) allows skipping encrypted frames *before* any payload inspection. This is a critical optimization for promiscuous mode sniffers, as encrypted traffic dominates the airwaves, and scanning it for plaintext patterns (like EtherTypes) yields false positives and wastes CPU cycles.
**Action:** Always filter by the `Protected` bit when looking for plaintext management/control frames or EAPOL handshakes.

## 2025-05-18 - [O(1) Header Parsing vs Linear Scan]
**Learning:** 802.11 headers are variable length, but the length is deterministically calculable from the Frame Control bits (ToDS, FromDS, Subtype, Order). Replacing a linear scan for the LLC/SNAP header with an O(1) offset calculation yields significant speedups (~4.5x in benchmarks) and improves correctness by avoiding false positive matches in random data.
**Action:** Prefer precise offset calculations over linear scans for structured packet headers, even if the structure is complex.
