# NeonGhostESPCYD

> **Architect: Alejandro Ramírez**

An advanced IoT security and entertainment firmware for the ESP32 Cheap Yellow Display (CYD) that gamifies network auditing through an interactive digital pet system.

---

## 🚀 Overview
NeonGhostESPCYD is a specialized firmware designed for the ESP32 Cheap Yellow Display (CYD) that bridges the gap between network monitoring and interactive entertainment. By combining real-time WiFi/BLE packet sniffing with a sophisticated 15-stage evolution engine, it transforms environmental signals into a progression-based experience where the digital "ghost" grows and evolves based on real-world security events.

## ✨ Key Features
- **Gamified Wardriving Engine:** Evolution-based pet system where progress is fueled by real-world network events such as handshakes, PMKIDs, and BLE discoveries.
- **15-Stage Evolution Path:** Advanced progression system spanning from 'Spark' to 'Daemon', featuring level-dependent attribute scaling and dynamic visual changes.
- **Real-Time Signal Analysis:** Integrated WiFi packet sniffer and BLE scanner for environmental auditing, signal tracking, and automated "XP" harvesting.
- **Hardware-Accelerated Rendering:** High-frame-rate creature animations and touch-driven UI specifically optimized for the ILI9341 display and XPT2046 touch controller.
- **Persistent State Management:** Robust SD card integration for storing pet statistics, level data, and discovery logs, ensuring long-term continuity.

## 🛠️ Tech Stack
- **Language:** C++
- **Framework:** Arduino (PlatformIO)
- **Hardware:** ESP32 (Cheap Yellow Display / CYD)
- **Libraries:**
  - `TFT_eSPI`: High-performance graphics rendering.
  - `NimBLE-Arduino`: Optimized Bluetooth Low Energy stack.
  - `XPT2046_Touchscreen`: Precise touch input management.
  - `SD`: Persistent storage and asset management.

## 📦 Installation & Getting Started

### Prerequisites
- **VS Code** with the **PlatformIO IDE** extension installed.
- **ESP32 Cheap Yellow Display (CYD)** hardware.
- A microSD card (formatted to FAT32) for asset storage and data persistence.

### Setup
1. **Clone the repository:**
   ```bash
   git clone https://github.com/alexdrgpy06/NeonGhostESPCYD.git
   cd NeonGhostESPCYD
   ```

2. **Initialize Project:**
   Open the folder in VS Code. PlatformIO will automatically download the necessary toolchains and libraries.

3. **Configure Hardware (Optional):**
   Adjust the pin definitions in `platformio.ini` if you are using a different hardware revision.

4. **Flash the Firmware:**
   Connect your ESP32 CYD via USB and click the **Upload** button in the PlatformIO toolbar (or run `pio run -t upload` via CLI).

5. **Prepare SD Card:**
   Ensure your SD card contains the necessary assets (if applicable) and is inserted into the CYD slot before booting.

## 📜 License
This project is licensed under the **MIT** License.

---
*Built with precision by Alejandro Ramírez.*
