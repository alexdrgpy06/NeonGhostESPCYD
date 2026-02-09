## 2024-05-23 - Packet Parsing Optimization
**Learning:** High-frequency ISR/callback paths (like promiscuous mode packet handlers) are extremely sensitive to O(N) operations. Replacing a linear scan with a calculated O(1) offset for parsing standard headers (like 802.11+LLC/SNAP) significantly reduces CPU cycles per packet.
**Action:** When parsing network protocols, always attempt to calculate offsets based on header flags rather than scanning for magic bytes, as scanning is both slower and prone to false positives.
