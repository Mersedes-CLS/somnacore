#include "session.h"
#include "../net/api_client.h"
#include "../calib/calibrator.h"
#include <cstring>

void Session::setNfcUid(const char* uid) {
    if (uid) {
        strncpy(nfcUid_, uid, sizeof(nfcUid_) - 1);
        nfcUid_[sizeof(nfcUid_) - 1] = '\0';
    } else {
        nfcUid_[0] = '\0';
    }
}

void Session::feedDistance(uint16_t dist) {
    // Accumulate distance only when NOT in a set (rest/idle).
    // These pre-set samples reflect the resting weight stack position
    // and give a more accurate weight determination than in-motion samples.
    if (state_ == SessionState::IN_SET) return;
    distBuf_[distBufIdx_] = dist;
    distBufIdx_ = (distBufIdx_ + 1) % DIST_BUF_SIZE;
    if (distBufCount_ < DIST_BUF_SIZE) distBufCount_++;
}

void Session::feedRep(int16_t romMm) {
    if (curReps_ == 0) {
        setStartTime_ = millis();
        // Keep pre-set distance buffer — it already has resting samples
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

    // Determine weight from pre-set distance buffer
    int weightKg = -1;
    if (calibrator_ && distBufCount_ > 0) {
        weightKg = calibrator_->distToWeightKg(distBuf_, (uint8_t)distBufCount_);
    }

    // Send completed set to backend (with NFC UID if available)
    const char* uid = nfcUid_[0] ? nfcUid_ : nullptr;
    net::sendSet(curReps_, curRom_, millis() - setStartTime_, weightKg, uid);

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
    nfcUid_[0] = '\0';
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
