## 2024-05-15 - Immediate Visual Feedback on CYD Touch
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons. The hardware TFT does not have built-in hover/active states like DOM elements.
**Action:** When drawing interactive buttons on TFT screens, provide a helper function that takes a "pressed" state, or manually redraw the button inverted briefly when touched before executing the action.
