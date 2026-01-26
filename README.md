# NeonGhost 👻
**The Definitive Cyberpunk Hacker Pet for ESP32 CYD**

NeonGhost is an evolutionary "virtual pet" that lives on the WiFi & BLE spectrum. Inspired by Pwnagotchi, Marauder, and Tamagotchi, it feeds on network packets, evolves as it gains experience, and unlocks advanced hacking capabilities as it grows.

![Evolution Chart](https://via.placeholder.com/800x200?text=SPARK+%E2%86%92+BABY+%E2%86%92+TEEN+%E2%86%92+SPECTER+%E2%86%92+REAPER+%E2%86%92+GLITCH+%E2%86%92+DEMON+%E2%86%92+GOD)

## ✨ Features

### 👾 Artificial Life
*   **XP System:** Gains XP for every handshake, new network, BLE device, or packet captured.
*   **Evolution:** Starts as a tiny energy spark and evolves through **8 distinct forms** (Spark, Baby, Teen, Specter, Reaper, Glitch, Demon, God).
*   **Personality:** Expresses emotions (Happy, Alert, Eating) via animations.
*   **Persistence:** Auto-saves progress to flash memory.

### 🔓 Hacking Capabilities (Unlockable)
NeonGhost gains new skills as it levels up (Tech Level):
*   **Level 1 (Passive):** Sniffs WiFi traffic on Channel 1.
*   **Level 4 (Hunter):** Unlocks **Channel Hopping** to scan the entire 2.4GHz spectrum.
*   **Level 10 (Poltergeist):** Unlocks **Autonomous Deauth** ("Hungry Mode"). If no handshakes are found recently, it will zap a random network to force a handshake capture.
*   **SD Card Support:** Saves captured PCAP files for analysis in Wireshark.

### 📱 Hardware
Designed for the **ESP32 CYD (Cheap Yellow Display) - Model CYD2432S028R**.
*   **Display:** 2.8" ILI9341 (320x240)
*   **Touch:** XPT2046
*   **Storage:** MicroSD Card Slot

## 🧬 Evolutionary Stages

| Stage | Name | Level Range | Special Trait |
| :--- | :--- | :--- | :--- |
| 1 | **SPARK** | 1-3 | Basic form. Pure energy. |
| 2 | **BABY** | 4-6 | Cute ghost. Unlocks **Channel Hopping**. |
| 3 | **TEEN** | 7-9 | Hooded figure. Faster XP gain. |
| 4 | **SPECTER** | 10-14 | Elegant form. Unlocks **Auto-Deauth**. |
| 5 | **REAPER** | 15-19 | Aggressive, scythe-wielding form. |
| 6 | **GLITCH** | 20-24 | Corrupted data entity. Harder to track. |
| 7 | **DEMON** | 25-29 | Final physical form. Horned & angry. |
| 8 | **GOD** | 30+ | Pure golden energy. The singularity. |

## 🛠️ Installation

1.  **Clone the Repo:**
    ```bash
    git clone https://github.com/yourusername/NeonGhost.git
    cd NeonGhost
    ```
2.  **Open in PlatformIO:**
    Ensure you have VS Code with the PlatformIO extension installed.
3.  **Flash:**
    Connect your ESP32 CYD via USB and click "Upload".

## ⚠️ Disclaimer
**NeonGhost is a proof-of-concept educational tool.**
The deauthentication features ("Hungry Mode") involve sending raw 802.11 management frames which may interfere with wireless networks.
*   Use only on networks you own or have permission to test.
*   The developers assume no liability for misuse.

## 🤝 Credits & Inspiration
Built on the shoulders of giants:
*   [Pwnagotchi](https://pwnagotchi.ai/) - The original AI hacking pet.
*   [ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder) - Comprehensive offensive tool.
*   [Ghost ESP](https://github.com/Spooks4576/Ghost_ESP) - Initial aesthetic inspiration.
*   [Tamaguino](https://github.com/alojzjakob/Tamaguino) - Tamagotchi logic on Arduino.
*   [ESP32-DIV](https://github.com/cifertech/ESP32-DIV) - Interface design ideas.

---
*Feed the Ghost. Rule the Spectrum.*
