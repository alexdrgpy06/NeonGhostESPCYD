## 2024-05-29 - Immediate Touch Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Implemented drawSingleButton helper to dynamically center text and handle normal/pressed (inverted) color states. Apply this to all interactive hardware UI elements going forward to confirm registration immediately upon touch.
