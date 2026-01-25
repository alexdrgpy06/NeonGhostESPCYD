#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

class SDManager {
public:
    SDManager();
    // Initialize SD card. Defaults to HSPI pins as requested: SCK=14, MISO=12, MOSI=13, SS=15.
    // Note: Some CYD boards use VSPI (18, 19, 23, 5) for the onboard SD slot.
    // If initialization fails with default, try passing standard VSPI pins.
    bool begin(uint8_t sck = 14, uint8_t miso = 12, uint8_t mosi = 13, uint8_t ss = 15);

    // Saves a packet to capture.pcap
    void savePCAP(uint8_t* payload, int length);

private:
    SPIClass* _spi;
    const char* FILENAME = "/capture.pcap";

    void writeGlobalHeader(File& file);
};

#endif
