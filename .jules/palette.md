## 2024-06-23 - Immediate Touch Feedback for TFT Displays
**Learning:** In embedded hardware (ESP32 CYD), buttons often feel unresponsive because there's no immediate visual feedback upon pressing them, leading users to double-tap or feel uncertain if their input was registered. Web-like CSS `:active` states don't exist here.
**Action:** Always provide immediate visual feedback on hardware touchscreens by momentarily redrawing the pressed button with inverted colors or a fill, followed by a short delay (e.g., `delay(50)`), before executing the action.
