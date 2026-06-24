## 2024-06-24 - Hardware Button Immediate Visual Feedback
**Learning:** For physical hardware touchscreens, users often cannot tell if a press registered correctly without physical tactile feedback, leading to multiple unintended taps. Immediate visual feedback (e.g. inverted button colors for ~50ms) is essential.
**Action:** Always implement a momentary color inversion loop (pressed state) when building custom C++ TFT button components before running the action logic.
