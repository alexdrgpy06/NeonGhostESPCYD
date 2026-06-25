## 2026-06-25 - Immediate Touch Feedback
**Learning:** Physical touchscreens (like TFT on ESP32 CYD) lack the inherent tactile feedback of physical buttons. Users often repeatedly tap the screen if the system takes time to respond, leading to unintended multiple actions.
**Action:** Always provide immediate visual feedback (e.g., momentarily redrawing a button with inverted colors using a short delay, ~50ms) as the very first action in touch event handlers before processing logic.
