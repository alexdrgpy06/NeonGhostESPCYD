## 2024-05-24 - Touch Feedback Pattern
**Learning:** Immediate visual feedback (press-wait-release) is critical for touch interfaces on MCUs where processing might block the UI thread.
**Action:** Use a helper function to draw the button in a pressed state, delay briefly (50ms), then redraw the unpressed state before executing the action.
