#include "pn532.h"
#include "../config.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Arduino.h>

namespace nfc {

static Adafruit_PN532 pn532(PIN_NFC_IRQ, PIN_NFC_RESET, &Wire);
static bool ready = false;

bool init() {
    pn532.begin();
    uint32_t ver = pn532.getFirmwareVersion();
    if (!ver) {
        Serial.println("[NFC] PN532 not found");
        return false;
    }
    Serial.printf("[NFC] PN532 FW v%d.%d\n", (ver >> 16) & 0xFF, (ver >> 8) & 0xFF);
    pn532.SAMConfig();
    ready = true;
    return true;
}

bool readTag(char* uid, uint8_t maxLen, uint16_t timeoutMs) {
    if (!ready || maxLen < 3) return false;
    uint8_t uidBuf[7];
    uint8_t uidLen = 0;
    if (!pn532.readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuf, &uidLen, timeoutMs)) {
        return false;
    }
    int pos = 0;
    for (uint8_t i = 0; i < uidLen && pos + 2 < maxLen; i++) {
        pos += snprintf(uid + pos, maxLen - pos, "%02X", uidBuf[i]);
    }
    uid[pos] = '\0';
    return true;
}

} // namespace nfc
