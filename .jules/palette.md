## 2024-07-04 - [Button Touch Feedback]
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** When implementing touch interactions on TFT screens, ensure a momentary visual change (like redrawing the button inverted) happens immediately before executing the underlying logic.
