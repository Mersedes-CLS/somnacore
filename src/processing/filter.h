#pragma once

#include <cstdint>
#include "../config.h"

class MedianFilter {
public:
    MedianFilter(uint16_t minVal = DIST_MIN, uint16_t maxVal = DIST_MAX);

    // Feed a raw reading, returns filtered value or 0xFFFF on error
    uint16_t update(uint16_t raw);
    void     reset();

private:
    uint16_t buf_[FILTER_SIZE];
    uint8_t  idx_;
    bool     full_;
    uint16_t minVal_;
    uint16_t maxVal_;

    static uint16_t medianOfThree(uint16_t a, uint16_t b, uint16_t c);
};
