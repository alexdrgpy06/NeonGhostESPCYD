## 2026-02-08 - Interactive Button Feedback
**Learning:** Touchscreens on low-power devices like CYD lack tactile feedback. Users often "double tap" because they aren't sure if their touch was registered.
**Action:** Implemented a generic `drawSingleButton` helper that supports a `pressed` state (inverted colors). Used this in `handleTouch` to provide a 50ms visual flash before executing actions. This pattern prevents duplicate inputs and improves perceived responsiveness.
