## 2026-05-26 - EAPOL Payload Inspection Bottleneck
**Learning:** Encrypted 802.11 data frames cannot contain valid plaintext EAPOL headers. Inspecting the payload of these frames is a massive bottleneck that can be avoided by checking the 'Protected' bit (`packet[1] & 0x40`). This skip yields a ~22x to 47x speedup.
**Action:** Always check the 'Protected' bit before attempting to inspect 802.11 data frames for plaintext headers, and ensure a minimum length bounds check (e.g., `if (len >= 26)`) is used before accessing EAPOL header bytes to prevent out-of-bounds access.
