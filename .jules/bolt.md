## 2024-10-24 - [ISR Packet Optimization]
**Learning:** In high-frequency ISRs (like WiFi promiscuous mode), optimizing for the success path (detecting a rare packet) can unintentionally penalize the common path (ignoring 99% of traffic) if not careful. Adding "fast path" checks before a linear search adds overhead to every non-matching packet.
**Action:** Replace linear searches with O(1) checks at known offsets entirely. Do not fall back to linear search unless absolutely necessary and the performance cost is justified. Optimize for early rejection.
