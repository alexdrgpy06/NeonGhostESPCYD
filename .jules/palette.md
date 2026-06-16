## 2024-05-18 - Immediate Touch Feedback
**Learning:** Immediate visual feedback on physical touchscreens is critical to prevent users from repeatedly tapping buttons. When users don't see immediate feedback, they tap multiple times, which can trigger unintended side effects.
**Action:** When adding or modifying interactive UI components on TFT screens, always draw an inverted button or flash its background color momentarily (e.g., using a short `delay(50)`) inside the touch handling routine before running the main action.
