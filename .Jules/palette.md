## 2024-05-20 - Adding interactive button states
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. This UX principle is documented in .Jules/palette.md.
**Action:** Use a "press-wait-release" pattern implemented directly in handlers (like `handleTouch`), calling `drawSingleButton` with `pressed=true`, waiting ~50ms, and then redrawing with `pressed=false`.
