## 2024-05-15 - Immediate Visual Feedback on Physical Touchscreens
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. Furthermore, touch boundaries must strictly check both upper and lower bounds to prevent accidental activations from outside the intended area.
**Action:** Implement a 'press-wait-release' pattern for interactive buttons and ensure strict bounding box checks (both Y-min and Y-max).
