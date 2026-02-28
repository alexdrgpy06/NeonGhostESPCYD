# Palette's Journal

## 2024-05-26 - Physical Touchscreen Interaction Feedback
**Learning:** On physical touchscreens like the ESP32 CYD, users tend to repeatedly tap buttons if there's no immediate visual feedback, as they can't feel the "click" and processing the action may take a few moments.
**Action:** Implement a "press-wait-release" visual pattern (drawing the button in an inverted/pressed state, waiting a short delay like 50ms, and redrawing in the unpressed state) directly within the touch handler for all primary interactions to provide immediate confirmation of input.
