/**
 * @file AppState.h
 * @brief System state machine + radio mode for NeonGhost.
 *
 * The ESP32 has a single radio and the BLE (NimBLE) / WiFi stacks cannot all run
 * promiscuously at once, so the app tracks a RadioMode and the AttackManager
 * switches stacks when a power needs a different one. The SystemState drives how
 * the pet reacts visually (e.g. ATTACKING -> red / trembling / strobe LED).
 */
#ifndef APP_STATE_H
#define APP_STATE_H

#include <Arduino.h>

enum SystemState : uint8_t {
    SYS_IDLE = 0,
    SYS_SCANNING,
    SYS_ATTACKING,
    SYS_EVOLVING,
    SYS_SLEEPING,
    SYS_DEAD
};

enum RadioMode : uint8_t {
    RADIO_OFF = 0,
    RADIO_BLE,        // NimBLE scan + advertising (default, with BT disruption)
    RADIO_WIFI        // WiFi scan / deauth (mutually exclusive with BLE promisc)
};

static const char* SYSTEM_STATE_NAMES[] = {
    "IDLE", "SCANNING", "ATTACKING", "EVOLVING", "SLEEPING", "DEAD"
};

class AppState {
public:
    SystemState state = SYS_IDLE;
    RadioMode   radio = RADIO_OFF;

    void set(SystemState s) { state = s; }
    bool is(SystemState s) const { return state == s; }
    const char* name() const { return SYSTEM_STATE_NAMES[state]; }

    // The pet looks hostile (red/tremble) while attacking.
    bool aggressive() const { return state == SYS_ATTACKING; }
};

#endif // APP_STATE_H
