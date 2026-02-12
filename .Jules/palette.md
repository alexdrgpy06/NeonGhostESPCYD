## 2024-05-23 - Visual Feedback for Touch
**Learning:** On resistive touchscreens like the CYD's XPT2046, touch detection can be finicky. Users rely heavily on visual confirmation since there is no haptic feedback. Adding a synchronous "pressed" state (color inversion) with a small delay (50ms) significantly improves the perceived responsiveness and confidence in the interaction.
**Action:** Implement `drawButton` helpers with built-in pressed states for all custom UI elements.
