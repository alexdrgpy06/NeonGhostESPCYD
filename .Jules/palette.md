## 2024-05-24 - Touchscreen Button Feedback
**Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical. Without it, users may repeatedly tap buttons on unresponsive devices like the ESP32-CYD, thinking the system didn't register the tap.
**Action:** Use a 'press-wait-release' drawing pattern inside the main touch interaction event loops (`handleTouch`) to show the user the tap was captured before executing any heavy backend logic or drawing the next screen.
