# NeonGhost: Ultimate Hacker Pet v7.0

**NeonGhost** is a sophisticated virtual pet for the ESP32-2432S028 (Cheap Yellow Display) that feeds on WiFi packets and Bluetooth signals. Inspired by the Flipper Zero and Pwnagotchi, it gamifies wireless auditing with a premium "cyberpunk" aesthetic.

![Status](https://img.shields.io/badge/Status-Active-brightgreen) ![Platform](https://img.shields.io/badge/Platform-ESP32%20CYD-yellow) ![Version](https://img.shields.io/badge/Version-v7.0-blue)

## What's New in v7.0

- **⚡ Instant Boot**: Replaced slow animated splash with fast progress bar
- **🎨 RGB LED Ambilight**: Physical LED syncs with Ghost's color in real-time
- **🌊 Smooth Animations**: Improved breathing, bouncing, and particle effects
- **🎭 Reactive Personality**: Ghost changes color and animation per event type
- **💀 Death System**: Fainted state with XP-cost revival instead of instant reset
- **🎯 Performance**: Optimized rendering pipeline, reduced CPU usage

## Features

### Visuals
- **Zero-Flicker Rendering**: 240px wide double-buffer for rock-solid 30FPS
- **11 Animation States**: Idle, Eating, Happy, Alert, Evolving, Attack, Sleeping, Hacking, Scanning, Critical, Death
- **Dynamic Event Colors**:
  - Handshake → Happy animation (Stage color)
  - New Network → Eating animation (Orange)
  - Deauth → Attack animation (Red)
  - Probe → Scanning animation (Cyan)
  - BLE Device → Hacking animation (Green)

### Gameplay
- **15 Evolution Stages**: Progress from `SPARK` to the ultimate `DAEMON`
- **WiFi Feeding**: Handshakes and networks restore HP/MP
- **Sustainable Survival**: Balanced decay rates, fair revival system
- **Persistent Stats**: Auto-saves to NVS every 5 minutes

### Hardware Integration
- **RGB LED Sync**: Back LED matches Ghost's current color
- **Touch Interface**: Responsive touch controls
- **SD Card Support**: Optional packet capture storage

### Abilities (Unlock as you level)
**WiFi Operations**
- `Probe Sniff`: Passive collection of probe requests
- `AP Spam`: Broadcasts fake SSIDs (Random & Rick Roll) with channel hopping
- `Deauth Attack`: Disrupts a random nearby network using broadcast deauth/disassoc with rotating reason codes
- `Handshake Cap`: Passive EAPOL capture
- `PMKID Grab`: Passive PMKID capture
- `Full Scan`: Aggressive channel hopping scan

**BLE Operations (Stateless)**
- `BLE Sniff`: Passive detection of nearby devices
- `Sour Apple`: iOS Pair Spam (Continuity)
- `Samsung Spam`: Android Fast Pair Spam (Watch/Buds)
- `Swift Pair`: Windows Pair Spam
- `AirTag Spam`: Fake AirTag beacons
- `BLE Flood`: Mass device emulation

## Hardware Required

- **ESP32-2432S028** ("Cheap Yellow Display" aka CYD)
- MicroSD Card (optional, for capturing PCAP files)

## Installation

1. Clone this repository
2. Open with **PlatformIO** (VS Code)
3. Build and Upload to your ESP32 CYD
4. Enjoy your new digital companion!

## Controls

| Area | Action |
|------|--------|
| Ghost | Tap to interact |
| FEED | Manual feeding |
| ATTACK | Trigger current ability |
| MENU | View abilities and stats |
| Revive | Only when Fainted (costs XP) |

## Evolution Tree

| Stage | Name | Levels |
|-------|------|--------|
| 1 | SPARK | 1-2 |
| 2 | BYTE | 3-4 |
| 3 | GHOST | 5-6 |
| 4 | SPECTER | 7-9 |
| 5 | PHANTOM | 10-12 |
| 6 | WRAITH | 13-15 |
| 7 | SHADE | 16-18 |
| 8 | REVENANT | 19-22 |
| 9 | BANSHEE | 23-26 |
| 10 | LICH | 27-30 |
| 11 | POLTERGEIST | 31-35 |
| 12 | VOID | 36-40 |
| 13 | NIGHTMARE | 41-47 |
| 14 | REAPER | 48-55 |
| 15 | DAEMON | 56+ |

## Tech Stack

- **Platform**: ESP32 (Espressif32 6.x)
- **Display**: TFT_eSPI
- **Touch**: XPT2046
- **Storage**: Preferences (NVS), SD Library
- **Bluetooth**: NimBLE
- **WiFi**: ESP-IDF 802.11 Sniffer API

## Inspo & Credits

Built on the shoulders of giants:

- [Tamaputer](https://github.com/mindovermiles262/tamaputer)
- [Murdy the Cat](https://github.com/terceranexus6/Murdy_the_cat)
- [DigimonVPet](https://github.com/Berational91/DigimonVPet)
- [Tamaguino](https://github.com/alojzjakob/Tamaguino)
- [ESP32Marauder](https://github.com/justcallmekoko/ESP32Marauder)
- [ESP32-TamaPetchi](https://github.com/CyberXcyborg/ESP32-TamaPetchi)
- [Ghost_ESP](https://github.com/Spooks4576/Ghost_ESP)

## License

MIT

---

*Made with 💚 by Alex R.*
