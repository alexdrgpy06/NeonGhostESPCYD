## 2024-05-15 - Touchscreen Visual Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Always provide inverted visual states and a small blocking delay or animation on custom C++ hardware UI buttons to signal registration.
