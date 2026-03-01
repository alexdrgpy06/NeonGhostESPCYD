## 2024-05-24 - O(1) EAPOL Parsing
**Learning:** Linear payload scanning for EAPOL detection is inefficient and blocks the core. O(1) direct offset calculations based on 802.11 header flags yield significant performance improvements over linear scanning.
**Action:** Always use explicit structure and flag checks for header offsets instead of blind scanning.
