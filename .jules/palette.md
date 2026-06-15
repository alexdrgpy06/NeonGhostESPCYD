## 2024-05-24 - TFT Button Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Always implement a pressed state for physical hardware UI elements using helper functions like `drawSingleButton` to ensure users perceive their interactions immediately before blocking logic executes.
