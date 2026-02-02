## 2024-05-23 - Heap Allocation in WiFi Callback
**Learning:** The `PacketSniffer` was allocating `String` objects inside `processPacket` (called from WiFi promiscuous callback). This is a high-risk operation for high-frequency ISRs/tasks, leading to heap fragmentation and potential crashes under load.
**Action:** Always use fixed-size char arrays (`char buf[N]`) and `snprintf`/`strncpy` for string handling in high-frequency callbacks or ISRs.
