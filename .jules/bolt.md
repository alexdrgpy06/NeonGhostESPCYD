## 2024-05-04 - Eliminate heap fragmentation in O(N) searches
**Learning:** Iterative `String` concatenation inside an unbounded `while (true)` loop on the ESP32 causes severe heap fragmentation. Using a stack-allocated `char[32]` buffer with `snprintf` is ~2x faster and avoids heap churn.
**Action:** Always prefer stack-allocated `char` arrays and `snprintf` over `String` concatenation for frequent or unbounded iterative string formatting operations.
