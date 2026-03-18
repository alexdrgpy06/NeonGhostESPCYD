## 2024-05-18 - Immediate Touch Feedback on Physical Displays
**Learning:** Users tend to repeatedly tap physical touchscreen buttons if there isn't immediate visual feedback, causing unintended multiple actions or confusion since there's no native OS-level "pressed" state like on mobile devices.
**Action:** Implement a 'press-wait-release' pattern directly in the touch handler (e.g., using a short `delay(50)` with inverted colors) to provide instant tactile-like visual confirmation before executing the action.
