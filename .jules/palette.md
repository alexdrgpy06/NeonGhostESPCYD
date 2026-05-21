## 2024-05-21 - Physical Touchscreen Button Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Always provide inverted-color momentary visual feedback states when building physical hardware interfaces, such as the `drawSingleButton` pattern.
