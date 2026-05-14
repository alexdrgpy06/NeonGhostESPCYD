## 2024-05-14 - Physical Touchscreen Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons, as hardware touch sensors lack the inherent tactile feedback of mechanical buttons.
**Action:** Implement momentary visual state inversion for all interactive UI elements upon touch before executing their primary logic.
