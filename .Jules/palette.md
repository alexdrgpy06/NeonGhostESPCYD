## 2024-05-22 - Tactile Feedback on Embedded Touchscreens
**Learning:** On resistive touchscreens (XPT2046) without haptic hardware, visual feedback (inverting colors) is critical for confirming input, as the physical sensation is often mushy or uncertain. A 50ms blocking delay provides a perceptible "click" feeling without disrupting background tasks significantly.
**Action:** Always implement a `drawButton(pressed=true/false)` helper and animate the press state before executing actions.
