## 2024-05-18 - Touchscreen Interaction Feedback & Bounds
**Learning:** Immediate visual feedback (e.g., momentary color inversion) on physical touchscreens is critical to prevent users from repeatedly tapping buttons, as there's no native tactile response. Furthermore, touch boundaries must be strictly checked (both upper and lower bounds) to avoid accidental activations from stray taps.
**Action:** Implement a `drawSingleButton` helper to manage button states and invert colors briefly during interaction, and explicitly enforce `< BUTTON_Y + BUTTON_H` bounds in touch handlers.
