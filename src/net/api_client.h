#pragma once

#include <cstdint>

namespace net {

void sendSet(uint8_t reps, uint16_t romMm, uint32_t durationMs, int weightKg = -1);

} // namespace net
