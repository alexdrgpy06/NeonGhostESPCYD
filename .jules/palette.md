## 2024-05-24 - Immediate Visual Feedback on Hardware Touch
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Created `drawSingleButton` to consolidate button rendering and explicitly redraw the button inverted on touch to provide tactile-like feedback before proceeding with logic.
