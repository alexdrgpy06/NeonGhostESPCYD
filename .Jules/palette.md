## 2025-02-23 - [Physical Touchscreen UI Feedback]
**Learning:** Physical touchscreens like the CYD's XPT2046 lack the hover states found on web apps. Users often repeat-tap buttons because there is no immediate visual confirmation that their touch registered before a heavy task (like animation or state transition) begins.
**Action:** Always provide immediate visual feedback within the touch handler for interactive elements (e.g., using a short `delay()` with an inverted color redraw) to prevent frustrating multi-tap behaviors.
