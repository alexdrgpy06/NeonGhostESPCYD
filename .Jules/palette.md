## 2024-05-18 - Physical Touchscreen Feedback
**Learning:** Immediate visual feedback (like a pressed state with inverted colors) on physical touchscreens is crucial to prevent users from repeatedly tapping buttons, as they may otherwise assume the touch was not registered. This makes the device feel much more responsive and reduces accidental multi-clicks.
**Action:** Always implement a brief visual "pressed" state (e.g., using `delay(50)` and redrawing the button) for interactive elements on physical touch displays.
