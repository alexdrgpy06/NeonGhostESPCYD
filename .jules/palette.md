## 2024-05-24 - Visual Touch Feedback on CYD Screen
**Learning:** On physical ESP32 CYD touchscreens, users often repeatedly tap buttons because there's no native "pressed" state, leading to accidental multiple triggers or confusion.
**Action:** Implemented a momentary visual feedback pattern by inverting button colors for ~50ms upon touch, providing immediate confirmation of the interaction.
