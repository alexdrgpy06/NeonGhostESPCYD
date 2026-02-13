## 2025-02-13 - [Manual Interaction Feedback]
**Learning:** This repo uses raw `TFT_eSPI` drawing for UI with no event loop for button states. Button press feedback (color inversion) must be manually implemented by redrawing elements in the touch handler.
**Action:** When adding interactive elements, always implement a state-aware drawing helper (e.g. `drawButton(..., active)`) and use `draw(true); delay(50); draw(false);` in the touch handler to provide tactile feedback.
