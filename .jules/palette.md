## 2024-08-18 - Immediate Touch Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Always implement pressed states on TFT interfaces using inverted foreground/background colors and a short delay loop before triggering the action to assure the user the touch was registered.
