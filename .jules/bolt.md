## 2026-05-25 - Packet Sniffer EAPOL Optimization
**Learning:** In WiFi packet sniffing, parsing every data frame for EAPOL signatures (0x88 0x8E) creates a significant CPU bottleneck. The vast majority of data frames are encrypted (indicated by the 'Protected' bit `packet[1] & 0x40`), which means they cannot contain valid plaintext EAPOL headers.
**Action:** Always check the 802.11 Protected frame bit and minimum length bounds before performing deep payload inspection to skip encrypted frames entirely, yielding massive performance gains.
