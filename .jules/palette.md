## 2026-05-26 - Add touch feedback to main buttons
**Learning:** Physical touchscreens lack native visual feedback when a user taps a button, which can lead to repeated taps or uncertainty. A momentary inversion of colors (e.g., ~50ms delay) provides an immediate and satisfying cue.
**Action:** Implemented a reusable drawSingleButton method to handle normal and pressed states by inverting text/background colors, and integrated this into the main touch loop.
