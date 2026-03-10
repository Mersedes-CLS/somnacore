#include "session.h"
#include "../net/api_client.h"
#include "../calib/calibrator.h"

void Session::feedDistance(uint16_t dist) {
    if (state_ != SessionState::IN_SET) return;
    distBuf_[distBufIdx_] = dist;
    distBufIdx_ = (distBufIdx_ + 1) % DIST_BUF_SIZE;
    if (distBufCount_ < DIST_BUF_SIZE) distBufCount_++;
}

void Session::feedRep(int16_t romMm) {
    if (curReps_ == 0) {
        setStartTime_ = millis();
        // Reset distance buffer at start of new set
        distBufIdx_ = 0;
        distBufCount_ = 0;
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

    // Determine weight from accumulated distance buffer
    int weightKg = -1;
    if (calibrator_ && distBufCount_ > 0) {
        weightKg = calibrator_->distToWeightKg(distBuf_, (uint8_t)distBufCount_);
    }

    // Send completed set to backend
    net::sendSet(curReps_, curRom_, millis() - setStartTime_, weightKg);

    curReps_ = 0;
    curRom_ = 0;
    lastRepTime_ = 0;
    distBufIdx_ = 0;
    distBufCount_ = 0;
    state_ = SessionState::REST;
}

void Session::reset() {
    state_ = SessionState::IDLE;
    curReps_ = 0;
    curRom_ = 0;
    setCount_ = 0;
    lastRepTime_ = 0;
    setStartTime_ = 0;
    distBufIdx_ = 0;
    distBufCount_ = 0;
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
