## 2024-06-03 - Avoid String Concatenation in O(N) Loops
**Learning:** Iterative String concatenation inside an O(N) loop (like searching for the next available filename) causes significant heap churn and fragmentation on memory-constrained embedded systems like the ESP32.
**Action:** Use stack-allocated char buffers (e.g., char[32]) and snprintf instead of String concatenation for repetitive operations.
