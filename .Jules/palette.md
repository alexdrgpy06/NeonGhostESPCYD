## 2026-02-12 - Immediate Touch Feedback
**Learning:** Users on touch screens expect immediate visual confirmation (color change/haptic) upon press, even before the action completes. Lack of this makes the interface feel sluggish or unresponsive.
**Action:** Always implement a "pressed" state for custom buttons that inverts colors or changes opacity, and hold it for a brief duration (e.g., 50ms) if the action is instant.
