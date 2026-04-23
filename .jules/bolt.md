## 2024-05-24 - [String Optimization]
**Learning:** Avoid `String` object instantiation in high-frequency loops (e.g., `drawWifiList`, `getNextFileName`) on ESP32 to prevent heap fragmentation; use stack-allocated `char` buffers and standard C-string functions instead.
**Action:** When working on performance, actively search for dynamic `String` creation and replace it with fixed-size `char` arrays, particularly when memory and allocation speed is critical.
