## 2024-05-15 - Immediate visual feedback on touch inputs
**Learning:** Physical touchscreens require immediate visual feedback (e.g., momentary color inversion) to prevent users from repeatedly tapping buttons, as hardware responses are less fluid than native mobile apps.
**Action:** Always implement a brief momentary redraw delay (~50ms) with inverted colors for button feedback before executing touch actions.
