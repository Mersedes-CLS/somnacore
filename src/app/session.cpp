#include "session.h"

void Session::feedRep(int16_t romMm) {
    if (curReps_ == 0) {
        setStartTime_ = millis();
    }
    curReps_++;
    if (static_cast<uint16_t>(romMm) > curRom_) {
        curRom_ = romMm;
    }
    lastRepTime_ = millis();
    state_ = SessionState::IN_SET;
}

void Session::tick() {
    if (curReps_ > 0 && lastRepTime_ > 0 &&
        millis() - lastRepTime_ >= SET_TIMEOUT_MS) {
        closeSet();
    }
}

void Session::closeSet() {
    if (setCount_ < MAX_SETS) {
        sets_[setCount_] = {
            curReps_,
            curRom_,
            millis() - setStartTime_,
            millis()
        };
    }
    setCount_++;
    curReps_ = 0;
    curRom_ = 0;
    lastRepTime_ = 0;
    state_ = SessionState::REST;
}

void Session::reset() {
    state_ = SessionState::IDLE;
    curReps_ = 0;
    curRom_ = 0;
    setCount_ = 0;
    lastRepTime_ = 0;
    setStartTime_ = 0;
}

const char* Session::stateStr() const {
    switch (state_) {
        case SessionState::IN_SET: return "IN_SET";
        case SessionState::REST:   return "REST";
        default:                   return "IDLE";
    }
}

String Session::statusJson() const {
    String json = "{\"state\":\"";
    json += stateStr();
    json += "\",\"reps\":";
    json += curReps_;
    json += ",\"rom\":";
    json += curRom_;
    json += ",\"set\":";
    json += setCount_ + (curReps_ > 0 ? 1 : 0);
    json += ",\"t\":";
    json += millis();
    json += "}";
    return json;
}
