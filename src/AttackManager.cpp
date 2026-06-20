/**
 * @file AttackManager.cpp
 * @brief Orchestrates BLE / BT-Classic / WiFi powers, radio mode and the default
 *        automatic BT+BLE loop. See plan: no gating, mastery grows with use.
 */
#include "AttackManager.h"

#define AUTO_INTERVAL 9000UL
#define AUTO_MP_FLOOR  35   // conserve energy; don't auto-attack below this

void AttackManager::init(PetStatsManager* ps, BLEScanner* bleScanner) {
    pet = ps;
    ble = bleScanner;
    btc.init();
    wifi.init();
    app.set(SYS_IDLE);
    app.radio = RADIO_BLE; // NimBLE is brought up by BLEScanner in setup()
}

const char* AttackManager::lastPowerName() const {
    if (lastPower < 0 || lastPower >= (int)POWER_COUNT) return "";
    return POWERS[lastPower].name;
}

// Switch the radio stack between BLE and WiFi (single-radio constraint).
void AttackManager::setRadio(RadioMode m) {
    if (app.radio == m) return;
    if (m == RADIO_WIFI) {
        // Leaving BLE: stop any BLE activity first.
        btc.stop();
        if (ble) ble->stopAttack();
        wifi.enableRadio();
    } else if (m == RADIO_BLE) {
        wifi.disableRadio();
    }
    app.radio = m;
}

void AttackManager::dispatch(uint8_t powerId) {
    const PowerDef& p = POWERS[powerId];
    uint8_t intensity = constrain(pet->stats.stage / 2, 1, 5);

    switch (p.kind) {
        case PK_BLE:
            setRadio(RADIO_BLE);
            switch (powerId) {
                case 6:  ble->sourApple();   break;
                case 7:  ble->swiftPair();   break;
                case 8:  ble->samsungSpam(); break;
                case 9:  ble->airTagSpam();  break;
                case 10: ble->bleFlood();    break;
                default: ble->bleFlood();    break;
            }
            attackDur = 15000;
            break;

        case PK_BTCLASSIC:
            setRadio(RADIO_BLE);
            if (ble) ble->stopAttack(); // BTC owns advertising while disrupting
            btc.startDisrupt(intensity);
            attackDur = 15000;
            break;

        case PK_WIFI:
            setRadio(RADIO_WIFI);
            if (powerId == 14) {            // Deauth Strike
                wifi.deauth(1 + intensity);
                attackDur = 8000;
            } else {                        // WiFi Scan / Beacon Spam (recon)
                wifi.scan();
                attackDur = 4000;
            }
            break;

        case PK_RECON:
        default:
            setRadio(RADIO_BLE);
            if (ble) ble->startScan();
            attackDur = 3000;
            break;
    }
}

bool AttackManager::runPower(uint8_t powerId, bool manual) {
    if (!pet || powerId >= POWER_COUNT) return false;
    int cost = pet->powerMpCost(powerId);
    if (pet->stats.mp < (uint8_t)cost) return false;

    pet->stats.mp -= cost;
    dispatch(powerId);

    // Mastery + route preference (manual counts stronger).
    pet->gainMastery(powerId, manual ? 2 : 1);
    Route pr = routeForArchetype(POWERS[powerId].home);
    if (pr < ROUTE_COUNT) pet->addAffinity(pr, manual ? 3 : 1);

    lastPower = powerId;
    attackStart = millis();
    app.set(SYS_ATTACKING);
    pet->stats.lastActivity = millis();
    if (pet->stats.isSleeping) pet->wake();
    return true;
}

void AttackManager::stopAll() {
    btc.stop();
    if (ble) ble->stopAttack();
    if (app.radio == RADIO_WIFI) setRadio(RADIO_BLE);
    if (app.state == SYS_ATTACKING) app.set(SYS_IDLE);
}

// Default automatic behaviour: alternate a BT-Classic disruption power and a BLE
// spam power so the pet is "running BT + BLE together" with no intervention.
void AttackManager::autoTick() {
    unsigned long now = millis();
    if (now - lastAuto < AUTO_INTERVAL) return;
    if (busy()) return;
    if (!pet || pet->stats.isSleeping) return;
    if (pet->stats.mp < AUTO_MP_FLOOR) return; // conserve energy until fed
    lastAuto = now;

    uint8_t powerId;
    if (autoToggle) {
        powerId = 2 + random(0, 4);    // BT Classic powers (ids 2..5)
    } else {
        powerId = 6 + random(0, 5);    // BLE spam powers (ids 6..10)
    }
    autoToggle = !autoToggle;
    runPower(powerId, false);
}

void AttackManager::loop() {
    // Drive module sub-loops.
    btc.loop();
    if (ble && app.radio == RADIO_BLE) ble->loop();

    // Auto-stop an active attack after its duration.
    if (app.state == SYS_ATTACKING && (millis() - attackStart > attackDur)) {
        stopAll();
    }

    autoTick();
}
