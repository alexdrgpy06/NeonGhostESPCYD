## 2024-05-23 - Optimization Regression - Hidden Dependencies
**Learning:** When optimizing a render loop by skipping frames (dirty check), ensure that side-effects—like updating state variables used by other systems (e.g., `lastDrawX` used by the particle emitter)—are either preserved or correctly handled.
**Action:** Before removing or skipping code blocks, trace usage of all modified variables to ensure no downstream logic depends on them updating every frame.
