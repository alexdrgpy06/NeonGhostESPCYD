## 2024-03-29 - [Added Visual Button Feedback]
**Learning:** Immediate visual feedback on touchscreens is critical to prevent repeated tapping, especially on devices with slower refresh rates or animation delays. Buttons lacking a "pressed" state make the UI feel unresponsive.
**Action:** Implement the press-wait-release pattern directly in interaction handlers for all interactive buttons, momentarily redrawing the button with inverted colors before executing the associated action.
