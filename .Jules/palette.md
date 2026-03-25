## 2024-05-24 - Touchscreen Visual Feedback & Boundaries
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. Touch interaction bounds checking for buttons must explicitly verify both the upper (`ty >= BUTTON_Y`) and lower (`ty <= BUTTON_Y + BUTTON_H`) boundaries to accurately restrict active tap zones.
**Action:** Always implement a 'press-wait-release' pattern for interactive UI buttons and explicitly check full bounding boxes for touch events.
