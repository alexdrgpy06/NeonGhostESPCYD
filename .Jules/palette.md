## 2026-03-01 - Touchscreen Interaction Feedback
**Learning:** Users tend to double-tap physical touchscreens (like the CYD) when there's no immediate visual feedback for a press, which can trigger unintended double actions.
**Action:** Always implement a 'press-wait-release' visual pattern (e.g., inverting button colors for 50ms) before executing the button's action to confirm the touch was registered.
