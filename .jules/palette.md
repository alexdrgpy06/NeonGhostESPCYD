## 2024-04-30 - Touch Interaction Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. Additionally, explicit upper and lower boundary checks are necessary for touch interactions.
**Action:** Use a `drawSingleButton` helper function that supports a pressed state, and apply it in touch event handlers with a small delay. Always enforce strict bounds (e.g., `ty >= Y && ty <= Y + H`) in touch validation.
