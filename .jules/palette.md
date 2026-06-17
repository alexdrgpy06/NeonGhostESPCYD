## 2024-10-24 - Touchscreen Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Apply `drawSingleButton` with an inverted pressed state and a 50ms delay for all physical TFT touchscreen buttons.
