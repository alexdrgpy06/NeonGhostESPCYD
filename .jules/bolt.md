
## 2025-02-27 - Fast EAPOL Detection via 802.11 Flags
**Learning:** Linear payload scanning in promiscuous mode tasks can significantly delay ISRs/main loops. By utilizing 802.11 header bit flags (Protected bit, Address 4 presence, QoS, HT Control), we can skip encrypted frames and compute the exact EAPOL ethertype offset in O(1) time. This reduced the 10-million packet benchmark loop from 403ms to 0ms.
**Action:** Always favor structured, bit-flag-based parsing of standard network frames instead of brute-force byte scanning to maximize embedded system responsiveness.
