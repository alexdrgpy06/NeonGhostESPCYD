## 2024-05-23 - Embedded UI Feedback
**Learning:** Embedded UIs (TFT_eSPI) lack native "active" states, making manual visual feedback (inversion/delay) critical for perceived responsiveness.
**Action:** Always implement a `drawButton(pressed)` helper and use a short blocking delay (100ms) on touch to confirm input before action.
