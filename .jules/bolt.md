## 2024-04-19 - Avoid String concatenation in loops on ESP32
**Learning:** Iterative `String` concatenation during O(N) operations (like searching for the next available file on an SD card) causes significant heap churn and fragmentation on memory-constrained systems like the ESP32.
**Action:** Use stack-allocated `char` buffers and `snprintf` for string formatting, especially inside loops, to eliminate dynamic memory allocation overhead and improve performance.
