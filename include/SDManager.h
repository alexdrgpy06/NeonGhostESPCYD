#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// SD Card Pins for CYD (HSPI)
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_CS 5

// PCAP Global Header Constants
#define PCAP_MAGIC 0xA1B2C3D4
#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4
#define PCAP_ZONE 0
#define PCAP_SIGFIGS 0
#define PCAP_SNAPLEN 65535
#define PCAP_NETWORK 105 // IEEE 802.11

// Buffer Settings
#define BUF_SIZE 32768 // 32KB Ring Buffer

struct PcapGlobalHeader {
    uint32_t magic_number;
    uint16_t version_major;
    uint16_t version_minor;
    int32_t  thiszone;
    uint32_t sigfigs;
    uint32_t snaplen;
    uint32_t network;
};

struct PcapPacketHeader {
    uint32_t ts_sec;
    uint32_t ts_usec;
    uint32_t incl_len;
    uint32_t orig_len;
};

class SDManager {
public:
    SDManager();
    bool begin();

    // ISR Safe: Copies data to Ring Buffer
    void addPacket(uint8_t* buf, uint32_t len);

    // Task Safe: Writes data from Ring Buffer to SD
    void processBuffer();

    void openNewPCAP();
    
    bool isReady;

private:
    SPIClass *spi;
    File pcapFile;
    
    uint8_t *buffer;

    // Ring Buffer Indices
    volatile uint32_t head; // Write index (ISR modifies)
    volatile uint32_t tail; // Read index (Task modifies)
    
    void writeGlobalHeader();
    String getNextFileName();
};

#endif
