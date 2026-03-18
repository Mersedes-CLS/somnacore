#pragma once

#include <cstdint>
#include <Arduino.h>
#include "../config.h"
#include "set_record.h"

namespace calib { class Calibrator; }

enum class SessionState : uint8_t { IDLE, IN_SET, REST };

class Session {
public:
    void     setCalibrator(calib::Calibrator* cal) { calibrator_ = cal; }
    void     setNfcUid(const char* uid);  // set current NFC user
    void     feedDistance(uint16_t dist);  // accumulate distance readings (pre-set rest)
    void     feedRep(int16_t romMm);      // called when RepDetector reports a rep
    void     tick();                       // call each loop — checks 30s timeout
    void     reset();

    SessionState state()       const { return state_; }
    const char*  stateStr()    const;
    uint8_t  currentReps()     const { return curReps_; }
    uint16_t currentRom()      const { return curRom_; }
    uint8_t  setCount()        const { return setCount_; }
    const SetRecord* history() const { return sets_; }
    String   statusJson()      const;

private:
    void closeSet();

    calib::Calibrator* calibrator_ = nullptr;
    SessionState state_     = SessionState::IDLE;
    uint8_t  curReps_       = 0;
    uint16_t curRom_        = 0;
    uint8_t  setCount_      = 0;
    uint32_t lastRepTime_   = 0;
    uint32_t setStartTime_  = 0;
    SetRecord sets_[MAX_SETS] = {};

    // Distance ring buffer for weight determination (pre-set rest samples)
    uint16_t distBuf_[DIST_BUF_SIZE] = {};
    uint8_t  distBufIdx_   = 0;
    uint16_t distBufCount_ = 0;

    // NFC user
    char     nfcUid_[16]   = {};
};
