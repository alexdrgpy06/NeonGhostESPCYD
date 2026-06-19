# NeonGhostESPCYD

> **DIGI v9.0** — Digimon-style branching evolution + Full-BT cyber-pet.

An IoT security + entertainment firmware for the ESP32 Cheap Yellow Display (CYD)
that gamifies Bluetooth auditing through an evolving digital pet inspired by
Digimon: it grows, branches and gains power based on what it *does*.

---

## ⚠️ Authorized / educational use only
This firmware contains dual-use wireless capabilities (BLE advertising spam,
best-effort 2.4 GHz disruption, on-demand WiFi scan/deauth). **Only use it on
devices and networks you own or are explicitly authorized to test.** Interfering
with equipment you do not own may be illegal in your jurisdiction.

**ESP32 radio reality:** a stock ESP32 *cannot* truly jam/deauth Bluetooth
Classic (BR/EDR) audio links, and the BLE stack used here (NimBLE) is BLE-only.
The "BT disruption / speaker" powers are *best-effort* 2.4 GHz advertising
saturation, framed as disruption — not a jammer. WiFi deauth via raw 802.11 TX
is best-effort and may be dropped by newer ESP-IDF. The single radio is
time-shared: BLE and WiFi do not run promiscuously at the same time.

## 🧬 Evolution model
- **5 archetypes (style lines):** GENESIS, JAMMER (BT disruption), SPAMMER (BLE
  spam), SNIFFER (recon/PCAP), STRIKER (WiFi).
- **10 stages per line.** Stages are mostly **additive** (growing aura/FX); the
  **base image changes at milestones 4, 8 and 10**, where the pet may also
  **jump to another line** semi-randomly, weighted by its dominant affinity.
- **No gating:** every power is always available. Stages grow visuals and
  **power mastery** (potency / lower MP).
- **Default = automatic BT + BLE.** Picking a power from the menu runs it *and*
  marks **preference** (feeds the route) — so you steer the evolution.
- **New Game Plus:** power mastery and lifetime counters (attacks, devices,
  line jumps, ages…) persist across jumps and reboots (NVS).

## ✨ Key Features
- **AttackManager + state machine:** all offensive/recon logic and radio-mode
  switching live in one module; the pet reacts (red / tremble / strobe LED)
  while attacking.
- **Branching evolution engine** (`EvolutionTree.h` + `PetStats`) with affinity
  routing and milestone line jumps.
- **Procedural + SD hybrid art:** runs immediately with procedural archetype
  silhouettes + auras; multicolor frames load from SD when present
  (`tools/sprite_forge.py` builds the SD tree).
- **Hardware-accelerated rendering** for the ILI9341 + XPT2046 CYD.
- **Persistent NG+ state** in NVS, with migration from old v8 saves.

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

5. **Prepare SD Card (optional):**
   The firmware boots and plays fully with procedural art even without an SD card.
   For multicolor sprites, generate the asset tree and copy it to the card root:
   ```bash
   # placeholder tree (no Pillow needed)
   python3 tools/sprite_forge.py scaffold ./sd_root --size 96
   # or convert your own PNGs
   python3 tools/sprite_forge.py convert face.png sd_root/sprites/jammer/stage_4_0.bin
   ```
   Layout: `/sprites/<archetype>/stage_<n>_<frame>.bin` where `<n>` ∈ {1,4,8,10}
   (milestone bases) and `<frame>` ∈ {0=neutral,1=blink,2=happy,3=angry}.

## 📜 License
This project is licensed under the **MIT** License.

---
*Built with precision by Alejandro Ramírez.*
