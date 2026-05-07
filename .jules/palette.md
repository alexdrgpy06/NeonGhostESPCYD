## 2025-02-23 - Visual Feedback for Physical Touches
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Implemented `drawSingleButton` with a `pressed` state to automatically invert colors and applied it to the main action buttons.