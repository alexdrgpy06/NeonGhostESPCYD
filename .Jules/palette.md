## 2024-05-22 - [Touch Feedback Pattern]
**Learning:** Users on embedded touchscreens lack physical tactile feedback. A simple visual inversion (swap bg/fg) for a short duration (50ms) effectively mimics a button press and confirms the input registry before the action executes.
**Action:** Use the 'draw-delay-draw' pattern for all interactive touch elements in the future.
