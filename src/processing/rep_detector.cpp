#include "rep_detector.h"
#include "../config.h"
#include <Arduino.h>

bool RepDetector::update(uint16_t dist) {
    if (!calibrated_) {
        calAccum_ += dist;
        calSamples_++;
        if (calSamples_ >= REP_STABLE_COUNT) {
            baseline_ = calAccum_ / calSamples_;
            calibrated_ = true;
        }
        return false;
    }

    float delta = baseline_ - dist;

    if (!inRep_ && delta > baseline_ * REP_ENTER_PCT) {
        inRep_ = true;
        repStartTime_ = millis();
        peakDelta_ = static_cast<int16_t>(delta);
        return false;
    }

    if (inRep_) {
        if (static_cast<int16_t>(delta) > peakDelta_) {
            peakDelta_ = static_cast<int16_t>(delta);
        }
        if (delta < baseline_ * REP_EXIT_PCT) {
            inRep_ = false;
            if (millis() - repStartTime_ >= REP_MIN_MS) {
                // Drift baseline when not in rep
                baseline_ = baseline_ * BASELINE_DECAY + dist * BASELINE_ADAPT;
                return true;  // rep completed
            }
        }
        return false;
    }

    // Not in rep — drift baseline
    baseline_ = baseline_ * BASELINE_DECAY + dist * BASELINE_ADAPT;
    return false;
}

void RepDetector::reset() {
    baseline_ = 0;
    inRep_ = false;
    repStartTime_ = 0;
    calSamples_ = 0;
    peakDelta_ = 0;
    calibrated_ = false;
    calAccum_ = 0;
}
