#include "SDManager.h"

SDManager::SDManager() {
    _spi = nullptr;
}

bool SDManager::begin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss) {
    if (_spi) {
        delete _spi;
    }
    _spi = new SPIClass(HSPI);
    _spi->begin(sck, miso, mosi, ss);

    // Attempt to mount SD card
    if (!SD.begin(ss, *_spi)) {
        Serial.println("SD Card Mount Failed");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }

    Serial.println("SD Card Initialized");
    return true;
}

void SDManager::writeGlobalHeader(File& file) {
    uint32_t magic = 0xa1b2c3d4;
    uint16_t major = 2;
    uint16_t minor = 4;
    int32_t zone = 0;
    uint32_t sigfigs = 0;
    uint32_t snaplen = 65535;
    uint32_t network = 105; // IEEE 802.11

    file.write((uint8_t*)&magic, 4);
    file.write((uint8_t*)&major, 2);
    file.write((uint8_t*)&minor, 2);
    file.write((uint8_t*)&zone, 4);
    file.write((uint8_t*)&sigfigs, 4);
    file.write((uint8_t*)&snaplen, 4);
    file.write((uint8_t*)&network, 4);
}

void SDManager::savePCAP(uint8_t* payload, int length) {
    if (!SD.totalBytes()) return; // Check if SD is valid roughly

    File file = SD.open(FILENAME, FILE_APPEND);
    if (!file) {
        // Try creating
        file = SD.open(FILENAME, FILE_WRITE);
        if (!file) {
            Serial.println("Failed to open file for writing");
            return;
        }
    }

    // Check if new file
    if (file.size() == 0) {
        writeGlobalHeader(file);
    }

    // Packet Header
    uint32_t ts_sec = millis() / 1000;
    uint32_t ts_usec = (millis() % 1000) * 1000;
    uint32_t incl_len = length;
    uint32_t orig_len = length;

    file.write((uint8_t*)&ts_sec, 4);
    file.write((uint8_t*)&ts_usec, 4);
    file.write((uint8_t*)&incl_len, 4);
    file.write((uint8_t*)&orig_len, 4);

    // Payload
    file.write(payload, length);

    file.close();
}
