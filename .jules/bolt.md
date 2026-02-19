## 2024-03-24 - [Manual Packet Parsing]
**Learning:** Linear scanning for byte patterns (like EtherType 0x888E) in network packets is ~2.4x slower than calculating O(1) offsets. In embedded environments processing high-frequency interrupts (WiFi promiscuous mode), manual header parsing is critical for performance and correctness (avoiding false positives in encrypted payloads).
**Action:** Always favor direct offset calculation over linear scanning for protocol parsing in hot paths.
