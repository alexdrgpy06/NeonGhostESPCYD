## 2024-05-20 - [Button Visual Feedback]
 **Learning:** Immediate visual feedback (e.g., momentarily redrawing with inverted colors using a short delay) on physical touchscreens is critical to prevent users from repeatedly tapping buttons.
 **Action:** Implement a 'press-wait-release' pattern directly in handleTouch for all main interactive buttons to ensure tactile-like responsiveness.