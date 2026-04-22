## 2024-04-21 - Prevent Heap Fragmentation on ESP32
**Learning:** `String` object instantiation and concatenation in high-frequency loops (like UI rendering in `drawWifiList` or O(N) searches like `SDManager::getNextFileName`) cause significant heap churn and fragmentation on the ESP32 architecture, leading to eventual instability and slower execution.
**Action:** Use stack-allocated `char` buffers (`char[21]` for SSIDs, `char[32]` for file names) combined with C-string functions (`strncpy`, `snprintf`) instead of `String` objects in all performance-critical loops.
