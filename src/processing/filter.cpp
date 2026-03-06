#include "filter.h"

MedianFilter::MedianFilter(uint16_t minVal, uint16_t maxVal)
    : idx_(0), full_(false), minVal_(minVal), maxVal_(maxVal) {
    for (uint8_t i = 0; i < FILTER_SIZE; i++) buf_[i] = 0;
}

uint16_t MedianFilter::medianOfThree(uint16_t a, uint16_t b, uint16_t c) {
    if (a > b) { uint16_t t = a; a = b; b = t; }
    if (b > c) { uint16_t t = b; b = c; c = t; }
    if (a > b) { uint16_t t = a; a = b; b = t; }
    return b;
}

uint16_t MedianFilter::update(uint16_t raw) {
    if (raw == 0xFFFF) return 0xFFFF;

    if (raw < minVal_ || raw > maxVal_) {
        if (full_) return medianOfThree(buf_[0], buf_[1], buf_[2]);
        return 0xFFFF;
    }

    buf_[idx_] = raw;
    idx_ = (idx_ + 1) % FILTER_SIZE;
    if (idx_ == 0) full_ = true;

    if (!full_) return raw;
    return medianOfThree(buf_[0], buf_[1], buf_[2]);
}

void MedianFilter::reset() {
    idx_ = 0;
    full_ = false;
    for (uint8_t i = 0; i < FILTER_SIZE; i++) buf_[i] = 0;
}
