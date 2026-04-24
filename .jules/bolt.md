## 2025-04-24 - String Instantiation in High-Frequency Loops
**Learning:** Instantiating `String` objects in high-frequency loops (like `drawWifiList` or O(N) searches like `getNextFileName`) causes severe heap fragmentation and unnecessary performance overhead on the ESP32. Creating substrings further exacerbates this issue.
**Action:** Replace `String` instantiation and manipulation with stack-allocated `char` arrays and standard C string functions (like `snprintf`, `strncpy`, `strcpy`) whenever formatting simple values or interacting with hardware/libraries that accept `const char*`.
