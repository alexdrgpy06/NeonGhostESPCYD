## 2024-04-12 - Physical Touchscreen Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. The "press-wait-release" pattern provides needed tactile assurance.
**Action:** Implement momentary visual state changes (like inverted colors with a ~50ms delay) directly in the touch handling logic for physical buttons.
