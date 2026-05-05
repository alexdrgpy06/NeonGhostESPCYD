## 2024-05-24 - Hardware Button Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. This UX principle is documented in memory.
**Action:** Extract a `drawSingleButton` helper that supports drawing a button in both "normal" and "pressed" (inverted) states, and use it to flash buttons momentarily on touch.
