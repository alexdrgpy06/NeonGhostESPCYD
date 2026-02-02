## 2026-02-02 - Visual Feedback on Resistive Touch
**Learning:** On resistive touch screens (common in embedded devices), physical tactile feedback is missing. Immediate visual feedback (like button inversion) is critical to confirm the "press" event to the user, distinguishing a successful tap from a missed one.
**Action:** Always implement a "pressed" state for custom UI buttons that persists for ~100ms before executing the action.
