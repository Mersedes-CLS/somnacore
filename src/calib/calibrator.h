#pragma once

#include <cstdint>
#include "../net/calib_client.h"

class VL53L0X;

namespace calib {

class Calibrator {
public:
    void begin(VL53L0X* sensor);
    void tick();  // call every main loop iteration

    // Load calibration table from server (call after WiFi connects)
    void loadFromServer();

    // Look up weight from distance buffer. Returns -1 if no calibration or out of range.
    // Takes ring buffer of distance readings collected during the set.
    int distToWeightKg(const uint16_t* buf, uint8_t count) const;

    // Rebuild midpoint boundaries after loading calibration table.
    void buildBoundaries();

private:
    static constexpr uint8_t  NUM_POS     = 17;
    static constexpr uint8_t  FIRST_KG    = 5;
    static constexpr uint8_t  STEP_KG     = 5;
    static constexpr uint8_t  SAMPLES     = 25;
    static constexpr uint16_t JITTER_WARN = 30;
    static constexpr uint16_t DELTA_WARN  = 15;

    struct CalPoint {
        uint16_t dist, dmin, dmax, jitter;
        bool valid;
    };

    VL53L0X*  sensor_      = nullptr;
    CalPoint  table_[NUM_POS] = {};
    uint8_t   currentPos_  = 0;
    bool      calibrating_ = false;
    bool      liveMode_    = false;
    uint32_t  liveTimer_   = 0;

    // Remote calibration state
    uint32_t  remotePushTimer_ = 0;
    uint32_t  remotePollTimer_ = 0;

    // Server-loaded calibration for weight lookup
    net::CalibPoint serverTable_[NUM_POS] = {};
    int serverTableCount_ = 0;

    // Sorted calibration points and midpoint boundaries
    struct SortedPoint {
        uint16_t dist;
        int      weightKg;
    };
    SortedPoint sorted_[NUM_POS] = {};
    uint16_t    boundaries_[NUM_POS - 1] = {};  // midpoints between sorted positions
    uint8_t     sortedCount_ = 0;

    uint8_t posKg(uint8_t i) const { return FIRST_KG + i * STEP_KG; }

    void printHelp();
    void printTable();
    void promptPos();
    bool doMeasure(uint8_t idx);
    void resetAll();
    void handleCommand(char cmd);
    void tickRemote();
};

}  // namespace calib
