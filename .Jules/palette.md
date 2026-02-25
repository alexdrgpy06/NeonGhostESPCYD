## 2026-02-25 - [Tactile Feedback Pattern]
**Learning:** Touch inputs on the CYD screen feel unresponsive without immediate visual feedback. A simple pattern of "Draw Pressed -> Delay(50ms) -> Draw Released" provides a satisfying "click" feel.
**Action:** Use the `drawSingleButton` helper with this pattern for all future interactive elements.
