#pragma once

#include <cstdint>
#include <Arduino.h>
#include "../config.h"
#include "set_record.h"

enum class SessionState : uint8_t { IDLE, IN_SET, REST };

class Session {
public:
    void     feedRep(int16_t romMm);    // called when RepDetector reports a rep
    void     tick();                     // call each loop — checks 30s timeout
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

    SessionState state_     = SessionState::IDLE;
    uint8_t  curReps_       = 0;
    uint16_t curRom_        = 0;
    uint8_t  setCount_      = 0;
    uint32_t lastRepTime_   = 0;
    uint32_t setStartTime_  = 0;
    SetRecord sets_[MAX_SETS] = {};
};
