## 2024-05-23 - [Optimization] EAPOL Detection
**Learning:** Packet processing loops (O(N)) in `promiscuous_rx_cb` or `processPacket` can be severe bottlenecks. Linear scanning for ethertypes (`88 8E`) is dangerous and slow.
**Action:** Always parse 802.11 headers using O(1) offset calculations (accounting for Address 4, QoS, HT Control) and check `Protected` bit to skip encrypted frames early.
