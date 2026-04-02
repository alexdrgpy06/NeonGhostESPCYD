## 2025-04-02 - Visual Feedback and Bounding on Physical Touchscreens
**Learning:** Physical touchscreens like the CYD often lack tactile feedback, making it easy for users to repeatedly tap a button or trigger unintended actions when bounds checking is loose (`if (ty >= BUTTON_Y)`).
**Action:** Always provide immediate visual feedback (e.g., a momentary color inversion loop) when a button is pressed, and explicitly verify both the upper and lower boundaries (`if (ty >= BUTTON_Y && ty <= BUTTON_Y + BUTTON_H)`) to accurately restrict tap zones.
