## 2024-05-14 - Visual Feedback on Physical Touchscreens
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Apply a momentary color inversion (`pressed = true`) to buttons upon touch detection before executing the associated action.
