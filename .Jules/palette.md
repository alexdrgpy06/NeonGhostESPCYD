# Palette's Journal

## 2024-05-22 - Visual Feedback on Resistive Touch
**Learning:** Resistive touchscreens (like XPT2046 on CYD) lack tactile feedback. Users often tap multiple times if there's no immediate visual response, leading to accidental double-actions.
**Action:** Implement "Press-Wait-Release" pattern: Invert button colors immediately on touch, hold for ~100ms, then release. This confirms input before the action completes.
