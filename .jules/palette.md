## 2024-05-10 - Button Visual Feedback
**Learning:** Physical touchscreens lack tactile feedback, making users unsure if a tap registered. Immediate visual feedback (like momentary color inversion) prevents double-taps and feels much more responsive.
**Action:** Always implement momentary state changes (`pressed` state) with a small delay (~50ms) for main action buttons on physical TFT screens.
