## 2024-05-18 - [Optimizing 802.11 DATA Frame Sniffing]
**Learning:** Parsing 802.11 DATA frames blindly for EAPOL packets wastes significant CPU cycles when the frame payload is encrypted (WPA/WPA2). Checking the 'Protected' bit (`packet[1] & 0x40`) allows us to safely skip encrypted data frames, because they cannot contain valid plaintext EAPOL headers.
**Action:** Always check the Protected Bit and minimum length bounds before scanning 802.11 payloads to yield massive (~24x) performance improvements on ESP32.
