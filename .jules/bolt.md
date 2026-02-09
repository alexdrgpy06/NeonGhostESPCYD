## 2024-05-22 - Optimizing Packet Parsing in ISRs
**Learning:** Linear searches in ISRs (like packet sniffing callbacks) are costly and can be replaced by O(1) offset calculations if the protocol structure is fixed (like 802.11). However, one must account for all variable header fields (QoS, WDS, HT Control) to maintain correctness.
**Action:** Always verify protocol spec details (like Order bit for HT Control) when converting linear search to offset calculation.
