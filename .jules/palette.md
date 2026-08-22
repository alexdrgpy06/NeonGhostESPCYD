## 2024-03-24 - [TFT Touchscreen Button Feedback]
 **Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
 **Action:** Refactor physical UI button drawing into a reusable `drawSingleButton` function that accepts a `pressed` state, and momentarily redraw the button in an inverted state when processing touch events to signal interaction.
