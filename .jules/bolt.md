## 2024-06-13 - Eliminate Heap Churn in O(N) String Construction
**Learning:** Iterative `String` concatenation within an O(N) loop (e.g., finding the next available filename) causes significant heap churn and fragmentation on the ESP32.
**Action:** Use `snprintf` into a stack-allocated buffer (e.g., `char[32]`) instead of dynamic `String` operations for repeated string constructions to improve memory stability.
