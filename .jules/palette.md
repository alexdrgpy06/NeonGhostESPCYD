## 2024-04-26 - Physical Hardware Button Interaction
**Learning:** ESP32 touchscreen buttons with `drawRoundRect` lack a native "pressed" or "active" state compared to CSS `:active`. When touched, they don't give visual feedback, leading users to multi-tap.
**Action:** Implement a short visual inversion (swap text and background color) within the `handleTouch` event, followed by a slight delay and a redraw, to simulate a tactile button press.
