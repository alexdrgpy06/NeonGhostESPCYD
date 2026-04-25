## 2024-04-26 - Physical Touchscreen Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. It's also important to enforce explicit lower/upper bounds checks (e.g. ty <= BUTTON_Y + BUTTON_H) to constrain touch zones correctly.
**Action:** Added `drawSingleButton` to `src/main.cpp` that accepts a `pressed` state to invert `bg` and `fg` colors upon tap.
