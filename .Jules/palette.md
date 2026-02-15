## 2024-05-21 - Touch Feedback Latency
**Learning:** On ESP32 CYD devices with resistive touch, the lack of immediate visual feedback makes the UI feel unresponsive, especially if the action logic takes >100ms (e.g., WiFi operations).
**Action:** Always implement a "press-wait-release" visual pattern (color inversion) for main action buttons before executing the business logic.
