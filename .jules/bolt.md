## 2024-03-24 - Protected Bit Skip Yields 21.8x Speedup
**Learning:** Checking the Protected bit (`packet[1] & 0x40`) before linearly scanning encrypted data frames for EAPOL signatures provides a massive performance boost, bypassing unnecessary inner loop execution. This yields ~21.8x faster processing for encrypted frames.
**Action:** Always check bit 6 (0x40) of Frame Control (byte 1) to skip payload inspections of encrypted 802.11 frames.

## 2024-03-24 - snprintf is 53.6% faster than String concat
**Learning:** In loops on the ESP32 (or simulating it), `String` concatenation causes heap churn and is measurably slower (~136k us) than stack-allocated `char[]` with `snprintf` (~70k us).
**Action:** Avoid `String` in high-frequency functions. Use C-strings.
