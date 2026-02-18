## 2024-05-22 - Optimized EAPOL Detection
**Learning:** Linear scanning for specific byte sequences (like `88 8E`) in network packets is inefficient and error-prone (false positives in encrypted data). 802.11 headers have variable lengths but are deterministic based on frame control bits.
**Action:** Always prefer O(1) header parsing using frame control bits (Protected, ToDS/FromDS, QoS, Order) to locate payloads instead of searching.
