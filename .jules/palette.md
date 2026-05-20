## 2026-05-20 - Immediate Visual Feedback on Physical Displays
**Learning:** Users lack tactile feedback on physical touchscreens (like the CYD). Without immediate visual response to interaction (such as color inversion), users often repeatedly tap the interface thinking it did not register, leading to double-activations.
**Action:** Always implement a momentary visual state change (e.g. inverted colors with ~50ms delay) upon touch detection *before* executing the heavy logic.
