## 2024-04-19 - String Allocations in O(N) ESP32 Loops
**Learning:** The `String` class on ESP32 causes heap allocations. Iterative operations like `"/capture_" + String(i) + ".pcap"` inside an O(N) loop (such as searching for the next available log file) continuously alloc and free memory, leading to heap fragmentation which can eventually cause a crash in long-running firmware.
**Action:** Use stack-allocated `char` arrays and `snprintf` for string formatting inside loops to guarantee deterministic memory usage and avoid heap fragmentation.
