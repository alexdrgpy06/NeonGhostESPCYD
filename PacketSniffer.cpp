#include "PacketSniffer.h"

SDManager* PacketSniffer::_sdManager = nullptr;
QueueHandle_t PacketSniffer::_packetQueue = nullptr;
bool PacketSniffer::_handshakeDetected = false;

// Fixed size packet structure for the queue to avoid dynamic allocation in ISR
struct RawPacket {
    uint16_t len;
    uint8_t payload[512]; // Enough for EAPOL
};

PacketSniffer::PacketSniffer() {}

void PacketSniffer::init(SDManager* sdManager) {
    _sdManager = sdManager;

    // Create a queue for 10 packets
    _packetQueue = xQueueCreate(10, sizeof(RawPacket));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    // Note: Assuming WiFi is already started/setup in NetworkScanner,
    // but promiscuous needs specific handling.
    // Ensure we are in STA mode (set in NetworkScanner)

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&PacketSniffer::snifferCallback);
}

bool PacketSniffer::hasHandshake() {
    return _handshakeDetected;
}

void PacketSniffer::clearHandshake() {
    _handshakeDetected = false;
}

void PacketSniffer::snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_DATA) return; // We only care about DATA frames for EAPOL (usually)
    // Sometimes EAPOL can be in WIFI_PKT_MISC depending on hardware/driver version, but usually DATA.

    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    uint8_t* data = pkt->payload;
    int len = pkt->rx_ctrl.sig_len; // Length of the packet without rx_ctrl

    if (len > 512) len = 512; // Cap length for our buffer
    if (len < 24) return;

    // Parse Frame Control
    uint8_t frame_control = data[0];
    uint8_t f_type = (frame_control >> 2) & 0x03;
    uint8_t f_subtype = (frame_control >> 4) & 0x0F;

    if (f_type != 2) return; // Ensure it is Data

    int header_len = 24;
    // Check for QoS Data (Subtype bit 3 is 1 for QoS)
    // 1000 (8), 1001 (9), 1010 (10), 1011 (11)...
    if ((f_subtype & 0x08)) {
        header_len += 2; // QoS Control
    }

    // Check if packet is long enough to contain LLC
    if (len < header_len + 8) return;

    uint8_t* llc = data + header_len;

    // Check SNAP: AA AA 03
    if (llc[0] == 0xAA && llc[1] == 0xAA && llc[2] == 0x03) {
        // Check EtherType: 88 8E (EAPOL)
        // LLC structure: DSAP(1) SSAP(1) Ctrl(1) OUI(3) EtherType(2)
        // Index:         0       1       2       3-5    6-7
        if (llc[6] == 0x88 && llc[7] == 0x8E) {
            // EAPOL Detected!
            RawPacket p;
            p.len = len;
            memcpy(p.payload, data, len);

            // Send to queue
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xQueueSendFromISR(_packetQueue, &p, &xHigherPriorityTaskWoken);
            if (xHigherPriorityTaskWoken) {
                // In FreeRTOS on ESP32, this might be needed,
                // but usually handled by SDK.
                // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
    }
}

void PacketSniffer::snifferLoop() {
    RawPacket packet;
    while (true) {
        if (xQueueReceive(_packetQueue, &packet, portMAX_DELAY) == pdTRUE) {
            if (_sdManager) {
                _sdManager->savePCAP(packet.payload, packet.len);
                _handshakeDetected = true;
                Serial.println("EAPOL Captured and Saved!");
            }
        }
    }
}

// Wrapper for xTaskCreate
void snifferTaskWrapper(void* parameter) {
    PacketSniffer* sniffer = (PacketSniffer*)parameter;
    sniffer->snifferLoop();
}
