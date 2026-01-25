#include "SDManager.h"

SDManager::SDManager() {
  isReady = false;
  buffer = NULL;
  head = 0;
  tail = 0;
  spi = new SPIClass(HSPI);
}

bool SDManager::begin() {
  // Allocate Ring Buffer
  if (buffer == NULL) {
    buffer = (uint8_t *)malloc(BUF_SIZE);
    if (buffer == NULL) {
      Serial.println("[SD] Failed to allocate buffer!");
      return false;
    }
  }

  // Init SPI and SD
  spi->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  if (!SD.begin(SD_CS, *spi, 40000000)) {
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
  if (!isReady) return;

  if (pcapFile) pcapFile.close();

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

// ISR Safe: Copies data to Ring Buffer
void SDManager::addPacket(uint8_t *packet, uint32_t len) {
  if (!buffer) return;

  uint32_t headerSize = sizeof(PcapPacketHeader);
  uint32_t totalSize = headerSize + len;

  // Calculate free space
  // Free space = (tail - head - 1 + BUF_SIZE) % BUF_SIZE
  // Note: We keep 1 byte empty to distinguish full vs empty (if head==tail)
  uint32_t freeSpace = (tail - head - 1 + BUF_SIZE) % BUF_SIZE;

  if (totalSize > freeSpace) {
    // Buffer Overflow - Drop Packet
    return;
  }

  // Prepare Header
  PcapPacketHeader header;
  unsigned long now = micros();
  header.ts_sec = now / 1000000;
  header.ts_usec = now % 1000000;
  header.incl_len = len;
  header.orig_len = len;

  // Copy Header
  // Handle Wrap-around
  if (head + headerSize <= BUF_SIZE) {
      memcpy(&buffer[head], &header, headerSize);
      head += headerSize;
  } else {
      // Split copy
      uint32_t part1 = BUF_SIZE - head;
      uint32_t part2 = headerSize - part1;
      memcpy(&buffer[head], &header, part1);
      memcpy(&buffer[0], ((uint8_t*)&header) + part1, part2);
      head = part2;
  }
  if (head == BUF_SIZE) head = 0; // Fix edge case if exact match

  // Copy Payload
  if (head + len <= BUF_SIZE) {
      memcpy(&buffer[head], packet, len);
      head += len;
  } else {
      uint32_t part1 = BUF_SIZE - head;
      uint32_t part2 = len - part1;
      memcpy(&buffer[head], packet, part1);
      memcpy(&buffer[0], packet + part1, part2);
      head = part2;
  }
  if (head == BUF_SIZE) head = 0;
}

// Task Safe: Writes data from Ring Buffer to SD
void SDManager::processBuffer() {
  if (!isReady || !pcapFile || !buffer) return;

  // Check available data
  // Available = (head - tail + BUF_SIZE) % BUF_SIZE
  uint32_t available = (head - tail + BUF_SIZE) % BUF_SIZE;

  if (available == 0) return;

  static unsigned long lastFlush = 0;
  size_t bytesWritten = 0;

  if (tail < head) {
      // Contiguous
      bytesWritten = pcapFile.write(&buffer[tail], available);
      tail += available;
  } else {
      // Wrapped
      uint32_t part1 = BUF_SIZE - tail;
      uint32_t part2 = head; // remaining from 0

      bytesWritten += pcapFile.write(&buffer[tail], part1);
      bytesWritten += pcapFile.write(&buffer[0], part2);

      tail = head;
  }

  if (tail == BUF_SIZE) tail = 0; // Should be handled by logic above but for safety

  // Flush periodically or if buffer was fullish
  if (millis() - lastFlush > 1000) { // Flush every 1 sec
      pcapFile.flush();
      lastFlush = millis();
  }
}
