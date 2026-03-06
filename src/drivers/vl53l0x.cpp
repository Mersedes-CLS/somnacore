#include "vl53l0x.h"
#include "../config.h"
#include "../hal/i2c_bus.h"
#include <Arduino.h>
#include <Wire.h>

VL53L0X::VL53L0X(uint8_t addr, uint8_t xshutPin)
    : addr_(addr), xshutPin_(xshutPin), stopVariable_(0) {
    pinMode(xshutPin_, OUTPUT);
    digitalWrite(xshutPin_, HIGH);
}

// --- Low-level register I/O with delays ---

uint8_t VL53L0X::readReg(uint8_t reg) {
    Wire.beginTransmission(addr_);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(addr_, (uint8_t)1);
    uint8_t val = Wire.read();
    delayMicroseconds(500);
    return val;
}

void VL53L0X::writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr_);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission(true);
    delayMicroseconds(500);
}

uint16_t VL53L0X::readReg16(uint8_t reg) {
    Wire.beginTransmission(addr_);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(addr_, (uint8_t)2);
    uint16_t val = (uint16_t)Wire.read() << 8;
    val |= Wire.read();
    delayMicroseconds(500);
    return val;
}

void VL53L0X::writeReg16(uint8_t reg, uint16_t val) {
    Wire.beginTransmission(addr_);
    Wire.write(reg);
    Wire.write((val >> 8) & 0xFF);
    Wire.write(val & 0xFF);
    Wire.endTransmission(true);
    delayMicroseconds(500);
}

// --- XSHUT power-cycle ---

void VL53L0X::xshutReset() {
    digitalWrite(xshutPin_, LOW);
    delay(XSHUT_LOW_MS);
    digitalWrite(xshutPin_, HIGH);
    delay(SENSOR_BOOT_MS);
}

// --- Probe ---

bool VL53L0X::probeAddress() {
    Wire.beginTransmission(addr_);
    return Wire.endTransmission() == 0;
}

// --- Calibration helper ---

bool VL53L0X::doSingleRef(uint8_t vhvInitByte) {
    writeReg(0x00, 0x01 | vhvInitByte);
    delay(10);

    uint32_t t0 = millis();
    while ((readReg(0x13) & 0x07) == 0) {
        if (millis() - t0 > CAL_TIMEOUT_MS) return false;
        delay(10);
    }
    writeReg(0x0B, 0x01);  // clear interrupt
    writeReg(0x00, 0x00);  // stop
    return true;
}

// --- Full 11-step init ---

bool VL53L0X::init() {
    // 1. Verify Model ID
    uint8_t model = readReg(0xC0);
    Serial.print("Model ID: 0x");
    Serial.println(model, HEX);
    if (model != 0xEE) {
        Serial.println("ERROR: not VL53L0X");
        return false;
    }
    delay(10);

    // 2. DataInit — set 2V8 mode
    uint8_t vhv = readReg(0x89);
    writeReg(0x89, vhv | 0x01);
    delay(10);

    // 3. Set I2C standard mode
    writeReg(0x88, 0x00);
    delay(10);

    // 4. Read stop_variable
    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    stopVariable_ = readReg(0x91);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);
    Serial.print("Stop variable: 0x");
    Serial.println(stopVariable_, HEX);
    delay(10);

    // 5. Disable MSRC and TCC limit checks
    writeReg(0x60, readReg(0x60) | 0x12);
    delay(10);

    // 6. Set signal rate limit to 0.25 MCPS
    writeReg16(0x44, 32);
    delay(10);

    // 7. Set sequence config
    writeReg(0x01, 0xE8);
    delay(10);

    // 8. GPIO config — new sample ready interrupt
    writeReg(0x0A, 0x04);
    uint8_t gpio_hv = readReg(0x84);
    writeReg(0x84, (gpio_hv & ~0x10) | 0x10);  // active low
    writeReg(0x0B, 0x01);  // clear interrupt
    delay(10);

    // 9. VHV calibration
    Serial.print("VHV cal... ");
    writeReg(0x01, 0x01);
    if (!doSingleRef(0x40)) { Serial.println("FAIL"); return false; }
    Serial.println("OK");
    delay(10);

    // 10. Phase calibration
    Serial.print("Phase cal... ");
    writeReg(0x01, 0x02);
    if (!doSingleRef(0x00)) { Serial.println("FAIL"); return false; }
    Serial.println("OK");
    delay(10);

    // 11. Restore full sequence config
    writeReg(0x01, 0xE8);
    delay(10);

    Serial.println("Init OK");
    return true;
}

// --- Single-shot measurement ---

uint16_t VL53L0X::readDistance() {
    // Preamble to set stop_variable
    writeReg(0x80, 0x01);
    writeReg(0xFF, 0x01);
    writeReg(0x00, 0x00);
    writeReg(0x91, stopVariable_);
    writeReg(0x00, 0x01);
    writeReg(0xFF, 0x00);
    writeReg(0x80, 0x00);
    delay(5);

    // Start single-shot range measurement
    writeReg(0x00, 0x01);

    // Wait for SYSRANGE_START bit 0 to clear
    uint32_t t0 = millis();
    while (readReg(0x00) & 0x01) {
        if (millis() - t0 > MEAS_TIMEOUT_MS) {
            Serial.println("Start timeout!");
            return 0xFFFF;
        }
        delay(5);
    }

    // Wait for result ready
    t0 = millis();
    while ((readReg(0x13) & 0x07) == 0) {
        if (millis() - t0 > MEAS_TIMEOUT_MS) {
            Serial.println("Meas timeout!");
            return 0xFFFF;
        }
        delay(5);
    }

    // Read range result
    uint16_t dist = readReg16(0x14 + 0x0A);

    // Clear interrupt
    writeReg(0x0B, 0x01);

    return dist;
}

// --- Full reset ---

bool VL53L0X::reset() {
    Serial.println("Resetting sensor...");
    xshutReset();
    hal::i2cBusRecovery();
    hal::i2cInit();

    if (!probeAddress()) {
        Serial.println("Sensor not responding after reset");
        return false;
    }
    return init();
}
