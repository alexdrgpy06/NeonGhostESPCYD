## 2024-05-23 - [Touch Feedback on Resistive Screens]
**Learning:** Resistive touch screens (like XPT2046 on CYD) lack the physical tactile feedback of capacitive glass or mechanical buttons. Users often press harder or repeatedly if there is no immediate visual confirmation.
**Action:** Always implement a "pressed" state (e.g., color inversion or brightness change) that triggers *immediately* upon detection, blocking for a short duration (~50ms) to ensure visibility before executing the action. This confirms the input was received.
