## 2024-05-24 - Touch Feedback Pattern
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
**Action:** Use a 'press-wait-release' pattern implemented directly in touch handlers, calling a button drawing helper with `pressed=true`, waiting a short delay (e.g. 50ms), and then redrawing with `pressed=false`. Ensure touch boundaries check both upper and lower limits.
