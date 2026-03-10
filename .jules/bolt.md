## 2024-05-17 - String Concatenation in Loops on ESP32
**Learning:** Using Arduino `String` object concatenation inside a tight `while` loop (like `fileName = "/capture_" + String(i) + ".pcap";`) causes significant heap churn and fragmentation due to repeated allocations and copies.
**Action:** Replace dynamic `String` allocations inside loops with stack-allocated `char` buffers and `snprintf()`, returning the final `String` only once at the end.

## 2024-05-17 - Volatile Pointer Access in ISR-Safe Code
**Learning:** Repeatedly reading and writing a `volatile` variable (like a ring buffer `head` index) during multi-step operations (e.g., `memcpy(&buffer[head], ...); head += size;`) forces the compiler to emit memory loads/stores every time, degrading performance and increasing the window for subtle race conditions if an interrupt fires midway.
**Action:** Always read `volatile` variables into local stack variables at the start of a function, perform all calculations and boundary checks against the local copy, and write back to the `volatile` global exactly once at the end.
