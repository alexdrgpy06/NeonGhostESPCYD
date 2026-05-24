## 2024-05-24 - Touch Feedback Pattern
**Learning:** For ESP32 hardware UI with resistive touch screens, immediate visual feedback (like momentary color inversion) is critical. Without it, users often repeatedly tap buttons because they aren't sure if the touch was registered, which can lead to accidental double-triggers (especially for actions like feeding or attacking).
**Action:** When adding or modifying interactive UI elements on TFT screens, always implement a short-duration visual state change (e.g., drawing an inverted button for ~50ms) before executing the action.
