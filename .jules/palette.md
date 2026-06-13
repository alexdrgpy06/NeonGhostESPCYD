## 2024-05-24 - Immediate Touchscreen Visual Feedback
**Learning:** Physical touchscreens lack the inherent hover/active states of web interfaces. Without immediate visual confirmation (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms), users often repeatedly tap buttons assuming the device didn't register the interaction.
**Action:** Always implement a momentary visual state change (pressed state) on custom hardware UI buttons before executing the button's action logic to ensure a responsive feel.
