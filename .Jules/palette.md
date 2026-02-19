# Palette's UX Journal

## 2024-10-24 - [Immediate Feedback on Touch]
**Learning:** The ESP32 CYD touchscreen interactions can feel unresponsive without immediate visual feedback because action execution (like animations) can have a slight delay or be subtle.
**Action:** Always implement a "pressed" state (e.g., color inversion) for custom UI buttons, even if it requires a short blocking delay (50ms) to ensure visibility before the action triggers.
