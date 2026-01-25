#include "SDManager.h"

SDManager::SDManager() {
  isReady = false;
  buffer = NULL;
  bufPtr = 0;
  lastFlushTime = 0;
  spi = new SPIClass(HSPI);
}

bool SDManager::begin() {
  // Allocate Buffer
  if (buffer == NULL) {
    buffer = (uint8_t *)malloc(BUF_SIZE);
    if (buffer == NULL) {
      Serial.println("[SD] Failed to allocate buffer!");
      return false;
    }
  }

  // Init SPI
  spi->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Init SD
  if (!SD.begin(SD_CS, *spi, 40000000)) { // 40MHz for faster writes
    Serial.println("[SD] Mount Failed");
    return false;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("[SD] No SD card attached");
    return false;
  }

  Serial.println("[SD] Initialized Successfully");
  isReady = true;

  openNewPCAP();

  return true;
}

String SDManager::getNextFileName() {
  int i = 0;
  String fileName;
  while (true) {
    fileName = "/capture_" + String(i) + ".pcap";
    if (!SD.exists(fileName)) {
      break;
    }
    i++;
  }
  return fileName;
}

void SDManager::openNewPCAP() {
  if (!isReady)
    return;

  if (pcapFile)
    pcapFile.close();

  String fname = getNextFileName();
  pcapFile = SD.open(fname, FILE_WRITE);

  if (pcapFile) {
    Serial.println("[SD] Opened: " + fname);
    writeGlobalHeader();
  } else {
    Serial.println("[SD] Failed to open " + fname);
  }
}

void SDManager::writeGlobalHeader() {
  PcapGlobalHeader header;
  header.magic_number = PCAP_MAGIC;
  header.version_major = PCAP_VERSION_MAJOR;
  header.version_minor = PCAP_VERSION_MINOR;
  header.thiszone = PCAP_ZONE;
  header.sigfigs = PCAP_SIGFIGS;
  header.snaplen = PCAP_SNAPLEN;
  header.network = PCAP_NETWORK;

  pcapFile.write((uint8_t *)&header, sizeof(header));
  pcapFile.flush();
}

void SDManager::addPacket(uint8_t *packet, uint32_t len) {
  if (!isReady || !pcapFile)
    return;

  // Check if packet fits in remaining buffer
  // Packet needs: Header (16 bytes) + Payload (len)
  uint32_t required = sizeof(PcapPacketHeader) + len;

  if (bufPtr + required >= BUF_SIZE) {
    saveBuffer(); // Flush current buffer
    if (required > BUF_SIZE) {
      // Packet too big for buffer even when empty? Skip it.
      return;
    }
  }

  // Construct Header
  PcapPacketHeader header;
  unsigned long now = micros();
  header.ts_sec = now / 1000000;
  header.ts_usec = now % 1000000;
  header.incl_len = len;
  header.orig_len = len;

  // Copy to Buffer
  memcpy(buffer + bufPtr, &header, sizeof(header));
  bufPtr += sizeof(header);

  memcpy(buffer + bufPtr, packet, len);
  bufPtr += len;

  // Auto-flush time check
  if (millis() - lastFlushTime > FLUSH_INTERVAL && bufPtr > 0) {
    saveBuffer();
  }
}

void SDManager::saveBuffer() {
  if (!isReady || !pcapFile || bufPtr == 0)
    return;

  size_t written = pcapFile.write(buffer, bufPtr);
  if (written != bufPtr) {
    Serial.println("[SD] Write Error!");
  }

  pcapFile.flush(); // Commit to SD
  bufPtr = 0;       // Reset buffer
  lastFlushTime = millis();
}
