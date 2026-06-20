/**
 * @file AttackManager.h
 * @brief Single owner of all offensive/recon logic + radio mode switching.
 *
 * Default behaviour = BT + BLE running automatically. Manual power requests
 * (from the menu) also mark route preference (feeding evolution) and grant extra
 * mastery. WiFi powers switch RadioMode (mutually exclusive with BLE promisc).
 */
#ifndef ATTACK_MANAGER_H
#define ATTACK_MANAGER_H

#include <Arduino.h>
#include "AppState.h"
#include "PetStats.h"
#include "BLEScanner.h"
#include "BTClassicManager.h"
#include "WiFiAttackManager.h"

class AttackManager {
public:
    AppState app;

    void init(PetStatsManager* ps, BLEScanner* bleScanner);
    void loop();

    // Returns true if the power was executed (enough MP). manual=true when the
    // player picked it from the menu (stronger preference + mastery).
    bool runPower(uint8_t powerId, bool manual);

    void stopAll();
    bool busy() const { return app.state == SYS_ATTACKING; }
    int  lastPowerId() const { return lastPower; }
    const char* lastPowerName() const;
    RadioMode radio() const { return app.radio; }

private:
    PetStatsManager*   pet  = nullptr;
    BLEScanner*        ble  = nullptr;
    BTClassicManager   btc;
    WiFiAttackManager  wifi;

    unsigned long attackStart = 0;
    unsigned long attackDur   = 0;
    int  lastPower = -1;
    unsigned long lastAuto = 0;
    bool autoToggle = false; // alternate BT / BLE in the default auto loop

    void setRadio(RadioMode m);
    void dispatch(uint8_t powerId); // performs the action + sets attackDur
    void autoTick();
};

#endif // ATTACK_MANAGER_H
