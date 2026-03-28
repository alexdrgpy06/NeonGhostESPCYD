# Bolt's Journal

## 2024-05-20 - Avoid Early Returns in processPacket
**Learning:** When optimizing the hot path in `PacketSniffer::processPacket` to skip encrypted data frames (by checking the Protected bit `packet[1] & 0x40`), adding an early `return` statement skips essential end-of-function logic like `savePacket` evaluations.
**Action:** Always wrap early-exit optimizations in standard `if` blocks instead of using early returns if there is downstream logic that needs to be preserved, ensuring functional correctness while maintaining performance gains.
