## 2026-03-10 - [Physical Touchscreen Button Feedback]
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons due to lack of haptic or visual confirmation.
**Action:** Implemented a 'press-wait-release' pattern using the `drawSingleButton` helper, calling it with `pressed=true`, waiting 50ms, and redrawing with `pressed=false` directly in the touch handler to provide immediate tactile-equivalent visual feedback.
