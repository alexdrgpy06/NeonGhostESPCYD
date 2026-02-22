## 2026-02-22 - [Touch Feedback]
**Learning:** Embedded touchscreens lack tactile feedback, leaving users uncertain if inputs were registered.
**Action:** Use the `drawButton(..., true)` (inverted) -> `delay(50)` -> `drawButton(..., false)` pattern for all interactive elements to provide clear visual confirmation.
