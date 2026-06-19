## 2024-06-03 - Heap Fragmentation in O(N) Searches
**Learning:** Iterative `String` concatenation within O(N) loop structures (like `SDManager::getNextFileName` checking existing files) causes severe heap churn and fragmentation on the ESP32.
**Action:** Replace `String` object usage inside such loops with stack-allocated `char` arrays (e.g., `char[32]`) and use `snprintf` for string formatting to eliminate intermediate allocations.
