## 2024-05-22 - Embedded UI Feedback
**Learning:** In touch-based embedded systems without an OS-level UI framework, users lack confidence in their actions if buttons don't visually respond.
**Action:** Always implement a manual "pressed" state redraw with a short delay (e.g., 100ms) for critical actions in the main loop.
