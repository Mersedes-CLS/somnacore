#pragma once

#include <cstdint>

class VL53L0X;

namespace calib {

class Calibrator {
public:
    void begin(VL53L0X* sensor);
    void tick();  // call every main loop iteration

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

    uint8_t posKg(uint8_t i) const { return FIRST_KG + i * STEP_KG; }

    void printHelp();
    void printTable();
    void promptPos();
    bool doMeasure(uint8_t idx);
    void resetAll();
    void handleCommand(char cmd);
};

}  // namespace calib
