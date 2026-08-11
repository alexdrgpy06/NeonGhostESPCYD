## 2024-08-11 - Physical Touchscreen Immediate Visual Feedback
**Learning:** Physical touchscreen interfaces on embedded hardware require immediate, momentary visual feedback (e.g. inverted colors for ~50ms) to confirm input, otherwise users repeatedly tap buttons causing unintended state changes.
**Action:** Always implement a momentary visual 'pressed' state on touch buttons before processing the logic on hardware UIs.
