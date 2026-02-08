## 2024-05-20 - [Missing Tactile Feedback]
**Learning:** The touch interface lacks immediate visual feedback (active states) when buttons are pressed, leading to user uncertainty about whether an action was registered.
**Action:** Implement a "press-wait-release" pattern for all touch interactions. When a button is tapped, invert its colors or show a pressed state, hold for 50ms, then revert. This mimics the tactile feel of physical buttons and provides crucial confirmation in a touch-only interface.
