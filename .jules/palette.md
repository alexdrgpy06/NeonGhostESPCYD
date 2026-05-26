## 2026-05-26 - Tactile visual feedback on touchscreens
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Always implement a brief "pressed" state redraw before triggering long or blocking interactions when working with physical displays.
