## 2024-05-22 - WiFi Packet Parsing Optimization
**Learning:** 802.11 packet parsing in promiscuous mode on ESP32 is a critical hot path. Linear scanning for EAPOL signatures (O(N)) in every data packet wastes significant CPU cycles, especially since most traffic is encrypted and can be skipped immediately by checking the "Protected" bit.
**Action:** Always prefer O(1) header parsing with early exits (like the Protected bit check) over linear scans for high-frequency packet processing.
