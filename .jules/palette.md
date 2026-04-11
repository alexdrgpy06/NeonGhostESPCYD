## 2024-05-24 - Hardware Touch UI Polish
**Learning:** Hardware touchscreens lack the implicit affordances of web pointers (like hover states). Without immediate visual feedback upon physical interaction, users repeatedly tap interactive areas, triggering unwanted redundant actions. Additionally, unbounded touch calculations can lead to phantom inputs from scrolling or off-screen touches.
**Action:** Always implement momentary visual press feedback (e.g., color inversion for 50ms) and enforce strict minimum/maximum boundary checks on X/Y axes when processing hardware touch inputs.
