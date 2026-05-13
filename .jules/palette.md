## 2024-05-13 - Visual Feedback on Touch
**Learning:** Physical touchscreens lack the tactile feedback of physical buttons. Without immediate visual feedback, users often repeatedly tap the same button, assuming their first input wasn't registered.
**Action:** Implemented momentary color inversion on tap (e.g., using a short ~50ms delay) to confirm touch registration and prevent accidental double-taps on hardware devices.
