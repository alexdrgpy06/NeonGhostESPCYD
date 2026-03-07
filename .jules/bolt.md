## 2025-02-27 - O(N) Array Scan in Deep Loop Bottleneck
**Learning:** Found a major performance bottleneck where the `PacketSniffer::processPacket` method used an O(N) loop to scan every byte of incoming 802.11 frames to search for EAPOL packets (`0x88 0x8E`). Because a vast majority (95%+) of data frames are encrypted, searching through the payload is completely wasted CPU cycles. Furthermore, directly checking `88 8E` introduces false positives.
**Action:** For low-level network parsers, always skip parsing encrypted payloads. Replaced the O(N) loop with an O(1) header offset calculation (accounting for Address 4, QoS, and HT Control variable lengths) combined with a fast-exit `!(packet[1] & 0x40)` to drop encrypted WPA2/WPA3 frames. Always include bounds checking `if (len >= offset + 8)` to prevent segfaults when calculating variable lengths.

## 2025-02-27 - Variable Scope Compilation Errors during Extraction
**Learning:** When extracting logic into nested blocks for performance optimization, variable scope issues are common. In `PacketSniffer::processPacket`, moving the `subtype` calculation into the inner `if (type == 2)` block caused compilation failures because it was used later down the line.
**Action:** Always run a compiler test (`pio run` or equivalent) immediately after implementing an optimization, even if it seems like a simple structure change, to verify variable scope is maintained.

## 2025-02-27 - Binary Artifact Bloat in Git
**Learning:** Committed a local C++ test harness binary (`tests/benchmark_sniffer`) to the repository. This is an anti-pattern as compiled binaries offer no value across different environments, bloat the `.git` directory, and present a security risk.
**Action:** Never commit compiled executables or objects. Always add them to `.gitignore` or explicitly delete them (`rm`) before finalizing changes.