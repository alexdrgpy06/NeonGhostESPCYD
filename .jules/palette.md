## 2024-05-18 - Immediate Touch Feedback
**Learning:** ESP32 CYD physical touch screens lack physical feedback. Users often repeatedly tap buttons if the screen doesn't respond instantly, leading to accidental double-taps or confusion.
**Action:** Always provide immediate visual feedback (like momentary color inversion) when a button is pressed, followed by a short delay (e.g., 50ms), before executing the action or returning to normal.
