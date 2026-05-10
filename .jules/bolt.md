## 2024-05-10 - PacketSniffer Loop Overhead on Encrypted Frames
**Learning:** Checking payload bytes inside the O(N) loop for every 802.11 data frame incurs massive overhead. It turns out encrypted frames (`packet[1] & 0x40`) mathematically cannot contain valid plaintext EAPOL headers anyway, so inspecting them is a waste.
**Action:** Always check the 'Protected' bit before attempting to parse payload headers, and implement a minimum length bounds check (e.g. `len >= 24`) before accessing those headers to get ~40x speedups.
