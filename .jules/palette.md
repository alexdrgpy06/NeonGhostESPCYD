## 2026-05-22 - Momentary Touch Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. C++ UIs require explicit button redraws instead of relying on CSS `:active` states.
**Action:** Always implement momentary inversion or visual changes during touch handling for main UI elements on embedded displays.
