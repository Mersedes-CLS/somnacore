#pragma once

#include <cstdint>

class RepDetector {
public:
    bool     update(uint16_t dist);   // feed distance, returns true if rep just completed
    void     reset();
    bool     isInRep()    const { return inRep_; }
    int16_t  peakDelta()  const { return peakDelta_; }
    uint16_t baseline()   const { return static_cast<uint16_t>(baseline_); }

private:
    float    baseline_    = 0;
    bool     inRep_       = false;
    uint32_t repStartTime_= 0;
    uint8_t  calSamples_  = 0;
    int16_t  peakDelta_   = 0;
    bool     calibrated_  = false;
    float    calAccum_    = 0;
};
