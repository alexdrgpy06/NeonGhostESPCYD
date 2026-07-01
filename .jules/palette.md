## 2024-05-24 - Touchscreen Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Abstract button rendering into a helper like `drawSingleButton` to easily trigger and revert inverted visual states when handling physical touch events.
