## 2024-06-10 - Button Press Visual Feedback
**Learning:** Immediate visual feedback on touchscreens is critical to prevent users from repeatedly tapping buttons, thinking their press wasn't registered.
**Action:** Always add a short delay (~50ms) with inverted colors when a physical button is pressed before executing its action to provide hardware-level touch confirmation.
