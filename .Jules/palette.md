## 2024-05-21 - Embedded Touch Feedback
**Learning:** On resistive touchscreens (like CYD), instantaneous logic execution feels unresponsive. A visual state inversion with a short blocking delay (100ms) provides necessary tactile confirmation.
**Action:** Always implement a press-wait-release visual cycle for touch buttons on embedded displays.
