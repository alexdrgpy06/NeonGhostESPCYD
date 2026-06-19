## 2024-05-18 - Immediate visual feedback on physical touchscreens
**Learning:** Hardware touchscreens require immediate, explicit visual feedback to register user intent and prevent them from repeatedly tapping the same control. Without hover states, users lack confidence that the UI received their input.
**Action:** Always provide instant visual feedback on button press (e.g., momentarily redrawing the button with inverted colors using a short ~50ms delay) before executing the main logic.
