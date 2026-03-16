## 2024-03-01 - Skipping Encrypted Frames
**Learning:** Checking the Protected bit (0x40) at packet offset 1 before scanning for EAPOL headers (0x888E) avoids expensive and unnecessary iteration over encrypted payloads.
**Action:** Always check the `Protected` bit and perform a bounds check when parsing 802.11 DATA frames.
