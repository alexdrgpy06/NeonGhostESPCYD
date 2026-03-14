## 2024-05-24 - Faster WPA Handshake Parsing by Skipping Encrypted Frames
**Learning:** Checking the Protected bit (`packet[1] & 0x40`) and safely skipping encrypted frames (with a basic bounds check `len >= 24`) before running a linear search for the EAPOL ether type avoids costly processing of illegible data frames, yielding a ~21.8x speedup.
**Action:** Always check the frame control and headers for optimization opportunities before inspecting the payload.
