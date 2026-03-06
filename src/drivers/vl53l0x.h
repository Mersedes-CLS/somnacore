#pragma once

#include <cstdint>

class VL53L0X {
public:
    VL53L0X(uint8_t addr, uint8_t xshutPin);

    void     xshutReset();           // power-cycle via XSHUT
    bool     probeAddress();         // returns true if sensor ACKs on its address
    bool     init();                 // full 11-step init sequence
    uint16_t readDistance();         // single-shot measurement, returns mm or 0xFFFF on error
    bool     reset();               // full reset: XSHUT cycle + bus recovery + re-init

private:
    uint8_t  addr_;
    uint8_t  xshutPin_;
    uint8_t  stopVariable_;

    uint8_t  readReg(uint8_t reg);
    void     writeReg(uint8_t reg, uint8_t val);
    uint16_t readReg16(uint8_t reg);
    void     writeReg16(uint8_t reg, uint16_t val);
    bool     doSingleRef(uint8_t vhvInitByte);
};
