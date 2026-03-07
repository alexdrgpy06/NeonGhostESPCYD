
## 2024-05-15 - Interactive UI buttons
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** The helper function `drawSingleButton` in `src/main.cpp` handles drawing button elements with a specific label, color, and pressed state (inverted colors). Interactive UI buttons in `src/main.cpp` utilize a 'press-wait-release' pattern implemented directly in `handleTouch`, calling `drawSingleButton` with `pressed=true`, waiting 50ms, and then redrawing with `pressed=false`.
