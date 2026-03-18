#pragma once

#include <cstdint>

namespace nfc {

bool init();

// Try to read an NFC tag. Returns true if a tag was read.
// uid is filled with hex string like "A1B2C3D4".
bool readTag(char* uid, uint8_t maxLen, uint16_t timeoutMs = 100);

} // namespace nfc
