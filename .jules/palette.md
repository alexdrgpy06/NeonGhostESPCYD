## 2024-10-18 - Immediate Button Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Always implement momentary state inversion on touch event handlers before processing the action to assure the user the tap was registered.
