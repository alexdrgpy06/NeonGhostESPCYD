## 2023-10-27 - Immediate Touch Feedback
**Learning:** In hardware UI with physical touch screens, providing immediate visual feedback upon touch (like momentarily redrawing the button with inverted colors) prevents users from repeatedly tapping buttons thinking their tap wasn't registered.
**Action:** When handling physical touch, immediately change the button state visually for a short duration (~50ms-150ms) to acknowledge the touch before processing long actions.
