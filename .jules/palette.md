## 2024-06-01 - Immediate touch feedback
**Learning:** For ESP32 C++ hardware UI projects with physical touchscreens, immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay, ~50ms) is critical to prevent users from repeatedly tapping buttons.
**Action:** When adding or updating buttons on a TFT touch screen, always implement a brief color inversion or flash state upon touch detection before executing the associated action.
