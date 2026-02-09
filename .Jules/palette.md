## 2024-05-22 - [Limited Screen Layout Verification]
**Learning:** For embedded UI changes where visual verification is impossible, creating a standalone C++ script to simulate layout logic and pixel placement is crucial to prevent overlaps and ensure elements fit within tight constraints (e.g., 240x320px).
**Action:** When modifying list views or dynamic layouts on small screens, write a quick C++ test script to calculate final Y coordinates and assert against screen bounds.
