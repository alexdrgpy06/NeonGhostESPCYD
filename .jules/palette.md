## 2024-05-24 - Hardware UI Interaction Feedback
**Learning:** Physical touch screens without tactile feedback lead to double-taps or confusion if visual state doesn't instantly change on press. Also, touch bounding boxes must be explicitly constrained on both upper and lower bounds to avoid accidental triggers across UI components.
**Action:** Implement a 'press-wait-release' visual pattern for all primary touch buttons (invert colors momentarily) and strictly bounds-check both `Y` min and max in touch handlers.
