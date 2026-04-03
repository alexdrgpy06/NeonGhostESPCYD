## 2024-05-24 - Touchscreen Button Feedback
**Learning:** Immediate visual feedback on physical touchscreens is critical to prevent users from repeatedly tapping buttons. When an action is taken on a touchscreen, a delay in rendering the next state can lead to user confusion.
**Action:** Implemented a 'press-wait-release' pattern for interactive UI buttons, where they are momentarily redrawn with inverted colors when tapped, before triggering the associated action.
