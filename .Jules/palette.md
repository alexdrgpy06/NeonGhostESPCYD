## 2024-10-24 - Tactile Feedback Pattern
**Learning:** In this custom TFT_eSPI UI, buttons lacked visual feedback. Created a reusable `drawMainButton` pattern that supports an inverted "pressed" state.
**Action:** For future interactive elements in this repo, use or extend `drawMainButton` (or similar helper) and implement the draw-delay-restore pattern in touch handlers to simulate tactile response.
