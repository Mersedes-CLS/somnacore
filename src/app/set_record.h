#pragma once

#include <cstdint>

struct SetRecord {
    uint8_t  reps;
    uint16_t romMm;        // max ROM in mm across all reps in set
    uint32_t durationMs;   // set duration
    uint32_t timestampMs;  // millis() when set ended
};
